
#include "piid.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>


static void ctrl_feed_forward(piid_t *controller, const float *rc_input)
{
    controller->f_local[1] += filter_run(controller->filter_feedforw_x, rc_input[0]);
    controller->f_local[2] += filter_run(controller->filter_feedforw_y, rc_input[1]);
    controller->f_local[3] += filter_run(controller->filter_feedforw_z, rc_input[2]);
}


static void ctrl_feed_forward_init(piid_t *controller, const float Ts)
{
    const float T = 1/(2*M_PI*FILT_FG);
    const float temp_a0 = (4*T*T + 4*FILT_D*T*Ts + Ts*Ts);
    
    const float a[2] = {
        (2*Ts*Ts - 8*T*T)/temp_a0,
        (4*T*T - 4*FILT_D*T*Ts + Ts*Ts)/temp_a0
    };
    
    /* x -axis */
    float b[3] = {
        (2*CTRL_JXX*(2*CTRL_TMC + Ts))/temp_a0,
        -(8*CTRL_JXX*CTRL_TMC)/temp_a0,
        (2*CTRL_JXX*(2*CTRL_TMC - Ts))/temp_a0
    };
    filter_init(controller->filter_feedforw_x,a,b,Ts);
    
    /* y -axis */
    b[0] = (2*CTRL_JYY*(2*CTRL_TMC + Ts))/temp_a0;
    b[1] = -(8*CTRL_JYY*CTRL_TMC)/temp_a0;
    b[2] = (2*CTRL_JYY*(2*CTRL_TMC - Ts))/temp_a0;
    filter_init(controller->filter_feedforw_y,a,b,Ts);
    
    /* z -axis */
    b[0] = (2*CTRL_JZZ*(2*CTRL_TMC + Ts))/temp_a0;
    b[1] = -(8*CTRL_JZZ*CTRL_TMC)/temp_a0;
    b[2] = (2*CTRL_JZZ*(2*CTRL_TMC - Ts))/temp_a0;
    filter_init(controller->filter_feedforw_z,a,b,Ts);
}

/**************************************************************************
 * PIID -Controller
 *************************************************************************/

int piid_init(piid_t *controller, const float Ts)
{
    controller->Ts = Ts;
    controller->int_enable = 1;
    controller->int_err1        = calloc(1,sizeof(adams4_t));
    controller->int_err2        = calloc(1,sizeof(adams4_t));
    controller->filter_lp_err   = calloc(1,sizeof(filt2nd_t));
    controller->filter_hp_err   = calloc(1,sizeof(filt2nd_t));
    
    controller->filter_feedforw_x = calloc(1,sizeof(filt2nd_full_t));
    controller->filter_feedforw_y = calloc(1,sizeof(filt2nd_full_t));
    controller->filter_feedforw_z = calloc(1,sizeof(filt2nd_full_t));
    
    if (
            (controller->int_err1 == NULL)||
            (controller->int_err2 == NULL)||
            (controller->filter_lp_err == NULL)||
            (controller->filter_hp_err == NULL)||
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
    
    filter_lp_init(controller->filter_lp_err , FILT_FG, FILT_D, Ts, 3);
    filter_hp_init(controller->filter_hp_err , FILT_FG, FILT_D, Ts, 3);
    
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

void piid_term(piid_t *controller)
{
    adams4_term(controller->int_err1);
    adams4_term(controller->int_err2);

    free(controller->xi_err);
    free(controller->f_local);
}

void piid_run(piid_t *controller, float *gyro_input, float *rc_input, float *u_ctrl)
{
    float error[3];
    float derror[3];
    
    error[0] = controller->ringbuf[controller->ringbuf_idx]   - gyro_input[0];
    error[1] = controller->ringbuf[controller->ringbuf_idx+1] - gyro_input[1];
    error[2] = controller->ringbuf[controller->ringbuf_idx+2] - gyro_input[2];
    
    controller->ringbuf[controller->ringbuf_idx]   = rc_input[0];
    controller->ringbuf[controller->ringbuf_idx+1] = rc_input[1];
    controller->ringbuf[controller->ringbuf_idx+2] = rc_input[2];
    
    controller->ringbuf_idx+=3;
    if (controller->ringbuf_idx>=3*CTRL_NUM_TSTEP)
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
    adams4_run(controller->int_err1,controller->xi_err,controller->Ts, controller->int_enable);
    
    /* 2nd error integration: */
    controller->int_err2->f0[0] = controller->xi_err[0];
    controller->int_err2->f0[1] = controller->xi_err[1];
    controller->int_err2->f0[2] = controller->xi_err[2];
    adams4_run(controller->int_err2, controller->xii_err, controller->Ts, controller->int_enable);
    
    /* controller: */            
    controller->f_local[1] = PIID_KP * error[0] + PIID_KI * controller->xi_err[0] + PIID_KII * controller->xii_err[0] + PIID_KD * derror[0];  
    controller->f_local[2] = PIID_KP * error[1] + PIID_KI * controller->xi_err[1] + PIID_KII * controller->xii_err[1] + PIID_KD * derror[1];
    controller->f_local[3] = PIID_Y_KP * error[2] +  PIID_Y_KI * controller->xi_err[2] + PIID_Y_KII * controller->xii_err[2] +  PIID_Y_KD * derror[2];
    
    /* compensation of NL */
    // controller->f_local[1] -= (CTRL_JYY-CTRL_JZZ)/CTRL_JXX*gyro_input[1]*gyro_input[2];
    // controller->f_local[2] -= (CTRL_JZZ-CTRL_JXX)/CTRL_JYY*gyro_input[0]*gyro_input[2];
    
    /* output: */
    u_ctrl[0] = controller->f_local[1];
    u_ctrl[1] = controller->f_local[2];
    u_ctrl[2] = controller->f_local[3];
    
     
    /* feed forward */
    ctrl_feed_forward(controller, rc_input);
}

