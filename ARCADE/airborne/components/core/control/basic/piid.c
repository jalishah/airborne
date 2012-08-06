

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <util.h>

#include "piid.h"
#include "../../filters/lowhi.h"


static void ctrl_feed_forward(piid_t *controller, const float rc[3])
{
   controller->f_local[1] += filter_full_run(controller->filter_feedforw_x, rc[0]);
   controller->f_local[2] += filter_full_run(controller->filter_feedforw_y, rc[1]);
   controller->f_local[3] += filter_full_run(controller->filter_feedforw_z, rc[2]);
}


static void ctrl_feed_forward_init(piid_t *controller, const float ts)
{
   const float T = 1.0f / (2.0f * M_PI * FILT_FG);
   const float temp_a0 = (4.0f * T * T + 4.0f * FILT_D * T * ts + ts * ts);

   const float a[2] = {
      (2.0f * ts * ts - 8.0f * T * T) / temp_a0,
      (4.0f * T * T - 4.0f * FILT_D * T * ts + ts * ts)/temp_a0};
    
   /* x -axis */
   float b[3] = {
       (2.0f * CTRL_JXX * (2.0f * CTRL_TMC + ts)) / temp_a0,
      -(8.0f * CTRL_JXX * CTRL_TMC) / temp_a0,
       (2.0f * CTRL_JXX * (2.0f * CTRL_TMC - ts)) / temp_a0
   };
   filter_full_init(controller->filter_feedforw_x,a,b,ts);
    
   /* y -axis */
   b[0] = (2*CTRL_JYY*(2*CTRL_TMC + ts))/temp_a0;
   b[1] = -(8*CTRL_JYY*CTRL_TMC)/temp_a0;
   b[2] = (2*CTRL_JYY*(2*CTRL_TMC - ts))/temp_a0;
   filter_full_init(controller->filter_feedforw_y,a,b,ts);
    
   /* z -axis */
   b[0] = (2*CTRL_JZZ*(2*CTRL_TMC + ts))/temp_a0;
   b[1] = -(8*CTRL_JZZ*CTRL_TMC)/temp_a0;
   b[2] = (2*CTRL_JZZ*(2*CTRL_TMC - ts))/temp_a0;
   filter_full_init(controller->filter_feedforw_z,a,b,ts);
}


int piid_init(piid_t *controller, const float ts)
{
   controller->ts = ts;
   controller->int_enable = 1;
   controller->int_err1 = calloc(1, sizeof(adams4_t));
   controller->int_err2 = calloc(1, sizeof(adams4_t));
   controller->filter_lp_err = calloc(1, sizeof(filt2nd_t));
   controller->filter_hp_err = calloc(1, sizeof(filt2nd_t));
    
   controller->filter_feedforw_x = calloc(1, sizeof(filt2nd_full_t));
   controller->filter_feedforw_y = calloc(1, sizeof(filt2nd_full_t));
   controller->filter_feedforw_z = calloc(1, sizeof(filt2nd_full_t));
    
   adams4_init(controller->int_err1, 3);
   adams4_init(controller->int_err2, 3);
    
   filter_lp_init(controller->filter_lp_err , FILT_FG, FILT_D, ts, 3);
   filter_hp_init(controller->filter_hp_err , FILT_FG, FILT_D, ts, 3);
    
   ctrl_feed_forward_init(controller, ts);
        
   controller->xi_err  = calloc(3, sizeof(float));
   controller->xii_err = calloc(3, sizeof(float));
   controller->f_local = calloc(4, sizeof(float));
    
   /* init ring-buffer: */
   for (int i = 0; i < 3 * CTRL_NUM_TSTEP; i++)
   {
      controller->ringbuf[i] = 0;
   }
   controller->ringbuf_idx = 0;
}


void piid_run(piid_t *controller, const float gyro[3], const float rc[3], float u_ctrl[3])
{
   float error[3];
   float derror[3];
 
   error[0] = controller->ringbuf[controller->ringbuf_idx]   - gyro[0];
   error[1] = controller->ringbuf[controller->ringbuf_idx+1] - gyro[1];
   error[2] = controller->ringbuf[controller->ringbuf_idx+2] - gyro[2];
 
   controller->ringbuf[controller->ringbuf_idx]   = rc[0];
   controller->ringbuf[controller->ringbuf_idx+1] = rc[1];
   controller->ringbuf[controller->ringbuf_idx+2] = rc[2];
 
   controller->ringbuf_idx += 3;
   if (controller->ringbuf_idx >= 3 * CTRL_NUM_TSTEP)
   {
      controller->ringbuf_idx = 0;
   }
 
   /* filters: */
   filter_hp_run(controller->filter_hp_err, error, derror);
   filter_lp_run(controller->filter_lp_err, error, error);
 
   /* error integration: */
   controller->int_err1->f0[0] = error[0];
   controller->int_err1->f0[1] = error[1];
   controller->int_err1->f0[2] = error[2];
   adams4_run(controller->int_err1,controller->xi_err,controller->ts, controller->int_enable);
 
   /* 2nd error integration: */
   controller->int_err2->f0[0] = controller->xi_err[0];
   controller->int_err2->f0[1] = controller->xi_err[1];
   controller->int_err2->f0[2] = controller->xi_err[2];
   adams4_run(controller->int_err2, controller->xii_err, controller->ts, controller->int_enable);

   /* controller: */            
   controller->f_local[1] = PIID_KP * error[0] + PIID_KI * controller->xi_err[0] + PIID_KII * controller->xii_err[0] + PIID_KD * derror[0];  
   controller->f_local[2] = PIID_KP * error[1] + PIID_KI * controller->xi_err[1] + PIID_KII * controller->xii_err[1] + PIID_KD * derror[1];
   controller->f_local[3] = PIID_Y_KP * error[2] +  PIID_Y_KI * controller->xi_err[2] + PIID_Y_KII * controller->xii_err[2] +  PIID_Y_KD * derror[2];
    
   /* compensation of NL */
   // controller->f_local[1] -= (CTRL_JYY-CTRL_JZZ)/CTRL_JXX*gyro[1]*gyro[2];
   // controller->f_local[2] -= (CTRL_JZZ-CTRL_JXX)/CTRL_JYY*gyro[0]*gyro[2];
    
   /* output: */
   FOR_EACH(i, u_ctrl)
   {
      u_ctrl[i] = controller->f_local[i];
   }
 
   /* feed forward */
   ctrl_feed_forward(controller, rc);
}

