
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <util.h>

#include "piid.h"
#include "../../filters/filter.h"


static void ctrl_feed_forward(piid_t *controller, float rc[3])
{
   float temp_ff;

   filter2_run(controller->filter_feedforw_x, &rc[0], &temp_ff);
   controller->f_local[1] = temp_ff;

   filter2_run(controller->filter_feedforw_y, &rc[1], &temp_ff);
   controller->f_local[2] = temp_ff;

   filter2_run(controller->filter_feedforw_z, &rc[2], &temp_ff);
   controller->f_local[3] = temp_ff;
}


static void ctrl_feed_forward_init(piid_t *controller, float Ts)
{
   float T = 1.0f / (2.0f * M_PI * FILT_FF_FG);
   float temp_a0 = (4.0f * T * T + 4.0f * FILT_FF_D * T * Ts + Ts*Ts);

   float a[2] = {
       (2.0f * Ts * Ts - 8.0f * T * T) / temp_a0,
       (4.0f * T * T - 4.0f * FILT_FF_D * T * Ts + Ts * Ts) / temp_a0
   };

   /* x -axis */
   float b[3] = {
       (2.0f * CTRL_JXX * (2.0f * CTRL_TMC + Ts)) / temp_a0,
       -(8.0f * CTRL_JXX * CTRL_TMC) / temp_a0,
       (2.0f * CTRL_JXX * (2.0f * CTRL_TMC - Ts)) / temp_a0
   };
   filter2_init(controller->filter_feedforw_x, a, b, Ts, 1);

   /* y -axis */
   b[0] = (2.0f * CTRL_JYY * (2.0f * CTRL_TMC + Ts)) / temp_a0;
   b[1] = -(8.0f * CTRL_JYY * CTRL_TMC) / temp_a0;
   b[2] = (2.0f * CTRL_JYY * (2.0f * CTRL_TMC - Ts)) / temp_a0;
   filter2_init(controller->filter_feedforw_y, a, b, Ts, 1);

   /* z -axis */
   b[0] = (2.0f * CTRL_JZZ * (2.0f * CTRL_TMC + Ts)) / temp_a0;
   b[1] = -(8.0f * CTRL_JZZ * CTRL_TMC) / temp_a0;
   b[2] = (2.0f * CTRL_JZZ * (2 * CTRL_TMC - Ts)) / temp_a0;
   filter2_init(controller->filter_feedforw_z, a, b, Ts, 1);
}


int piid_init(piid_t *controller, float Ts)
{
   controller->Ts = Ts;
   controller->int_enable = 1;
   controller->int_err1        = (adams4_t* )calloc(1,sizeof(adams4_t));
   controller->int_err2        = (adams4_t* )calloc(1,sizeof(adams4_t));
   controller->filter_lp_err   = (Filter1* )calloc(1,sizeof(Filter1));
   controller->filter_hp_err   = (Filter1* )calloc(1,sizeof(Filter1));
   controller->filter_ref        = (Filter2*)calloc(1,sizeof(Filter2));
   controller->filter_feedforw_x = (Filter2* )calloc(1,sizeof(Filter2));
   controller->filter_feedforw_y = (Filter2* )calloc(1,sizeof(Filter2));
   controller->filter_feedforw_z = (Filter2* )calloc(1,sizeof(Filter2));

   if (
           (controller->int_err1 == NULL)||
           (controller->int_err2 == NULL)||
           (controller->filter_lp_err == NULL)||
           (controller->filter_hp_err == NULL)||
           (controller->filter_ref == NULL)||
           (controller->filter_feedforw_x == NULL)||
           (controller->filter_feedforw_y == NULL)||
           (controller->filter_feedforw_z == NULL))
   {
      return 0;
   }
   if ((adams4_init(controller->int_err1,3)==0)||adams4_init(controller->int_err2,3)==0)
   {
      return 0;
   }

   filter1_lp_init(controller->filter_lp_err , FILT_C_FG, Ts, 3);
   filter1_hp_init(controller->filter_hp_err , FILT_C_FG, Ts, 3);
   filter2_lp_init(controller->filter_ref, FILT_FF_FG,FILT_FF_D, Ts, 3);
   ctrl_feed_forward_init(controller, Ts);

   controller->xi_err  = (float *)calloc(3,sizeof(float));
   controller->xii_err = (float *)calloc(3,sizeof(float));
   controller->f_local = (float *)calloc(4,sizeof(float));

   if ((controller->xi_err == NULL)||(controller->xii_err == NULL)||(controller->f_local == NULL))
   {
      return 0;
   }

   // init ring-buffer
   int i;
   for (i=0; i < 3*CTRL_NUM_TSTEP; i++)
   {
       controller->ringbuf[i] = 0;
   }
   controller->ringbuf_idx = 0;
   return 1;
}


void piid_run(piid_t *controller, float gyro[3], float rc[4], float u_ctrl[3])
{
    float error[3];
    float derror[3];

    /* feed forward calculation */
    ctrl_feed_forward(controller, rc);

    /* filter reference signals */
    filter2_run(controller->filter_ref, rc, rc);

    error[0] = controller->ringbuf[controller->ringbuf_idx]   - gyro[0];
    error[1] = controller->ringbuf[controller->ringbuf_idx+1] - gyro[1];
    error[2] = controller->ringbuf[controller->ringbuf_idx+2] - gyro[2];

    controller->ringbuf[controller->ringbuf_idx]   = rc[0];
    controller->ringbuf[controller->ringbuf_idx+1] = rc[1];
    controller->ringbuf[controller->ringbuf_idx+2] = rc[2];

    controller->ringbuf_idx+=3;
    if (controller->ringbuf_idx>=3*CTRL_NUM_TSTEP)
    {
        controller->ringbuf_idx = 0;
    }

    //Filter    
    filter1_run(controller->filter_hp_err, error, derror);
    filter1_run(controller->filter_lp_err, error, error);

    /*
     *Integration
     */

    // Error Integration
    controller->int_err1->f0[0] = error[0];
    controller->int_err1->f0[1] = error[1];
    controller->int_err1->f0[2] = error[2];
    adams4_run(controller->int_err1,controller->xi_err,controller->Ts, controller->int_enable);

    // 2nd Error Integration
    controller->int_err2->f0[0] = controller->xi_err[0];
    controller->int_err2->f0[1] = controller->xi_err[1];
    controller->int_err2->f0[2] = controller->xi_err[2];
    adams4_run(controller->int_err2, controller->xii_err, controller->Ts, controller->int_enable);

    /* eval controller */
    u_ctrl[0] = PIID_KP * error[0] + PIID_KI * controller->xi_err[0] + PIID_KII * controller->xii_err[0] + PIID_KD * derror[0];
    u_ctrl[1] = PIID_KP * error[1] + PIID_KI * controller->xi_err[1] + PIID_KII * controller->xii_err[1] + PIID_KD * derror[1];
    u_ctrl[2] = PIID_Y_KP * error[2] +  PIID_Y_KI * controller->xi_err[2] + PIID_Y_KII * controller->xii_err[2] +  PIID_Y_KD * derror[2];

    /* compensation of NL */
    // controller->f_local[1] -= (CTRL_JYY-CTRL_JZZ)/CTRL_JXX*gyro_input[1]*gyro_input[2];
    // controller->f_local[2] -= (CTRL_JZZ-CTRL_JXX)/CTRL_JYY*gyro_input[0]*gyro_input[2];
    //
    controller->f_local[1] += u_ctrl[0];
    controller->f_local[2] += u_ctrl[1];
    controller->f_local[3] += u_ctrl[2];
}



void piid_controller_term(piid_t *controller)
{
   adams4_term(controller->int_err1);
   adams4_term(controller->int_err2);

   free(controller->xi_err);
   free(controller->f_local);
}

