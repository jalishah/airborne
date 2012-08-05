#include "controller.h"
/*#include "control_paramter.h"*/
#include <stdlib.h>
#include <string.h>
#include <math.h>

/*int  controller_init(ControllerData *controller, float sample_time)
{
    controller->Ts = sample_time;
    controller->int_obsv = (Adams4_f* )calloc(1,sizeof(Adams4_f));
    controller->int_ctrl = (Adams4_f* )calloc(1,sizeof(Adams4_f));
    controller->int_rc   = (Adams4_f* )calloc(1,sizeof(Adams4_f));
    
    if ((controller->int_obsv == NULL)||(controller->int_ctrl == NULL)||(controller->int_rc == NULL))
    {
        return 0;
    }
    if ((adams4_init(controller->int_obsv,3)==0)||(adams4_init(controller->int_ctrl,3)==0)||(adams4_init(controller->int_rc,3)==0))
    {
        return 0;
    }
    
    controller->ang_obsv = (float *)calloc(3,sizeof(float));
    controller->ang_rc   = (float *)calloc(3,sizeof(float));
    controller->omg_d   = (float *)calloc(3,sizeof(float));
    controller->xi_ctrl  = (float *)calloc(3,sizeof(float));
    controller->f_local  = (float *)calloc(4,sizeof(float));
    
    if ((controller->ang_obsv == NULL)||(controller->ang_rc == NULL)||(controller->xi_ctrl == NULL)||(controller->f_local == NULL)||(controller->omg_d == NULL))
    {
        return 0;
    }
    
    return 1;    
}

void controller_term(ControllerData *controller)
{
    adams4_term(controller->int_obsv);
    adams4_term(controller->int_ctrl);
    adams4_term(controller->int_rc);
    
    free(controller->ang_obsv);
    free(controller->ang_rc);
    free(controller->omg_d);
    free(controller->xi_ctrl);
    free(controller->f_local);
}

void controller_run(ControllerData *controller, const float *gyro_input, const float *rc_input)
{
    controller->int_rc->f0[0] = rc_input[0];
    controller->int_rc->f0[1] = rc_input[1];
    controller->int_rc->f0[2] = rc_input[2];
    adams4_run(controller->int_rc,controller->ang_rc,controller->Ts);
    
    observer_run(controller, gyro_input);

    pi_controller(controller, rc_input, gyro_input);
    
    outer_controller(controller);
    
    inner_controller(controller, rc_input, gyro_input);
}


void outer_controller( ControllerData *controller)
{
    const float error[3] = 
        {controller->ang_rc[0] - controller->ang_obsv[0],
         controller->ang_rc[1] - controller->ang_obsv[1],
         controller->ang_rc[2] - controller->ang_obsv[2]};
    controller->int_ctrl->f0[0] = error[0];
    controller->int_ctrl->f0[1] = error[1];
    controller->int_ctrl->f0[2] = error[2];
    adams4_run(controller->int_ctrl,controller->xi_ctrl,controller->Ts);
    
    controller->omg_d[0] = error[0] *CTRL_OUT_KP + controller->xi_ctrl[0] *CTRL_OUT_KI;
    controller->omg_d[1] = error[1] *CTRL_OUT_KP + controller->xi_ctrl[1] *CTRL_OUT_KI;
    controller->omg_d[2] = error[2] *CTRL_OUT_KP + controller->xi_ctrl[2] *CTRL_OUT_KI;
}

void inner_controller(ControllerData *controller, const float *rc_input, const float *gyro_input)
{
    controller->f_local[1] = -(CTRL_JYY-CTRL_JXX)/CTRL_JZZ*gyro_input[1]*gyro_input[2] + CTRL_LAMBDA_X * (controller->omg_d[0]+rc_input[0]-gyro_input[0]);
    controller->f_local[2] = -(CTRL_JZZ-CTRL_JXX)/CTRL_JYY*gyro_input[0]*gyro_input[2] + CTRL_LAMBDA_Y * (controller->omg_d[1]+rc_input[1]-gyro_input[1]);
    controller->f_local[3] = CTRL_LAMBDA_Z * (controller->omg_d[2]+rc_input[2]-gyro_input[2]);
}

void observer_run(ControllerData *controller, const float *gyro_input)
{
    /*const float cos_phi = cos(controller->ang_obsv[0]);
    * const float sin_phi = sin(controller->ang_obsv[0]);
    * const float cos_theta = cos(controller->ang_obsv[1]);
    * const float sin_theta = sin(controller->ang_obsv[1]);
    * controller->int_obsv->f0[0] = gyro_input[0] + sin_theta*gyro_input[2];
    * controller->int_obsv->f0[1] = cos_phi*gyro_input[1] - cos_theta*sin_phi*gyro_input[2];
    * controller->int_obsv->f0[2] = sin_phi*gyro_input[1] + cos_theta*cos_phi*gyro_input[2];
    controller->int_obsv->f0[0] = gyro_input[0];
    controller->int_obsv->f0[1] = gyro_input[1];
    controller->int_obsv->f0[2] = gyro_input[2];
    adams4_run(controller->int_obsv,controller->ang_obsv,controller->Ts);
}*/

/*void pi_controller(ControllerData *controller, const float *rc_input, const float *gyro_input)
{
    controller->f_local[1] = -0*(CTRL_JYY-CTRL_JXX)/CTRL_JZZ*gyro_input[1]*gyro_input[2] + CTRL_OUT_KP*(rc_input[0] - gyro_input[0]) + CTRL_OUT_KI*(controller->ang_rc[0] - controller->ang_obsv[0]);
    controller->f_local[2] = -0*(CTRL_JZZ-CTRL_JXX)/CTRL_JYY*gyro_input[0]*gyro_input[2] + CTRL_OUT_KP*(rc_input[1] - gyro_input[1]) + CTRL_OUT_KI*(controller->ang_rc[1] - controller->ang_obsv[1]);
    controller->f_local[3] = CTRL_OUT_KP*(rc_input[2] - gyro_input[2]) + CTRL_OUT_KI*(controller->ang_rc[2] - controller->ang_obsv[2]);
}*/

/*
int  pid_controller_init(PIDController *controller, const float Ts)
{
    controller->Ts = Ts;
    controller->int_err         = (Adams4_f* )calloc(1,sizeof(Adams4_f));
    controller->filter_lp_err   = (Filter2nd* )calloc(1,sizeof(Filter2nd));
    controller->filter_hp_err   = (Filter2nd* )calloc(1,sizeof(Filter2nd));
    
    controller->filter_feedforw_x = (Filter2ndFull* )calloc(1,sizeof(Filter2ndFull));
    controller->filter_feedforw_y = (Filter2ndFull* )calloc(1,sizeof(Filter2ndFull));
    controller->filter_feedforw_z = (Filter2ndFull* )calloc(1,sizeof(Filter2ndFull));
    
    if (
            (controller->int_err == NULL)||
            (controller->filter_lp_err == NULL)||
            (controller->filter_hp_err == NULL)||
            (controller->filter_feedforw_x == NULL)||
            (controller->filter_feedforw_y == NULL)||
            (controller->filter_feedforw_z == NULL))
    {
        return 0;
    }
    if (adams4_init(controller->int_err,3)==0)
    {
        return 0;
    }
    
    filter_lp_init(controller->filter_lp_err , FILT_FG, FILT_D, Ts, 3);
    filter_hp_init(controller->filter_hp_err , FILT_FG, FILT_D, Ts, 3);
    
    ctrl_feed_forward_init(controller, Ts);
        
    controller->xi_err  = (float *)calloc(3,sizeof(float));
    controller->f_local  = (float *)calloc(4,sizeof(float));
    
    if ((controller->xi_err == NULL)||(controller->f_local == NULL))
    {
        return 0;
    }
    
    // init ring-buffer
    int i;
    for (i=0; i < CTRL_NUM_TSTEP; i++)
    {
        controller->ringbuf[i];
    }
    controller->ringbuf_idx = 0;
    
    return 1;
}

void pid_controller_term(PIDController *controller)
{
    adams4_term(controller->int_err);

    free(controller->xi_err);
    free(controller->f_local);
}

void pid_controller_run(PIDController *controller, const float *gyro_input, const float *rc_input)
{
    float error[3];
    float derror[3];
    
    error[0] = controller->ringbuf[controller->ringbuf_idx][0] - gyro_input[0];
    error[1] = controller->ringbuf[controller->ringbuf_idx][1] - gyro_input[1];
    error[2] = controller->ringbuf[controller->ringbuf_idx][2] - gyro_input[2];
    
//     controller->test_out[0] = controller->ringbuf[controller->ringbuf_idx][0];
//     controller->test_out[1] = controller->ringbuf[controller->ringbuf_idx][1];
//     controller->test_out[2] = controller->ringbuf[controller->ringbuf_idx][2];
    
    controller->ringbuf[controller->ringbuf_idx = rc_input[0];
    controller->ringbuf[controller->ringbuf_idx+1] = rc_input[1];
    controller->ringbuf[controller->ringbuf_idx+2] = rc_input[2];
    
    controller->ringbuf_idx+=3;
    if (controller->ringbuf_idx>=3*CTRL_NUM_TSTEP)
    {
        controller->ringbuf_idx = 0;
    }
    
    
    filter_hp_run(controller->filter_hp_err, error, derror);
    
    filter_lp_run(controller->filter_lp_err, error, error);
    
    memcpy(controller->int_err->f0,error,3);
    adams4_run(controller->int_err,controller->xi_err,controller->Ts);
    
    controller->f_local[1] = PID_KP * error[0] + PID_KI * controller->xi_err[0] + PID_KD * derror[0];  
    controller->f_local[2] = PID_KP * error[1] + PID_KI * controller->xi_err[1] + PID_KD * derror[1];
    controller->f_local[3] = PID_KP * error[2] + PID_KI * controller->xi_err[2] + PID_KD * derror[2];
    
    ctrl_feed_forward(controller, rc_input);
}*/

/*************************************************************************
 * Feed-Forward
 ************************************************************************/
/*
void ctrl_feed_forward(PIDController *controller, const float *rc_input)
{
    controller->f_local[1] += filter_run(controller->filter_feedforw_x, rc_input[0]);
    controller->f_local[2] += filter_run(controller->filter_feedforw_y, rc_input[1]);
    controller->f_local[3] += filter_run(controller->filter_feedforw_z, rc_input[2]);
}*/

void ctrl_feed_forward_PIID(PIIDController *controller, const float *rc_input)
{
    controller->f_local[1] += filter_run(controller->filter_feedforw_x, rc_input[0]);
    controller->f_local[2] += filter_run(controller->filter_feedforw_y, rc_input[1]);
    controller->f_local[3] += filter_run(controller->filter_feedforw_z, rc_input[2]);
}

/*void ctrl_feed_forward_init(PIDController *controller, const float Ts)
{
    const float T = 1/(2*M_PI*FILT_FG);
    const float temp_a0 = (4*T*T + 4*FILT_D*T*Ts + Ts*Ts);
    
    const float a[2] = {
        (2*Ts*Ts - 8*T*T)/temp_a0,
        (4*T*T - 4*FILT_D*T*Ts + Ts*Ts)/temp_a0
    };
    //x-Axis
    float b[3] = {
        (2*CTRL_JXX*(2*CTRL_TMC + Ts))/temp_a0,
        -(8*CTRL_JXX*CTRL_TMC)/temp_a0,
        (2*CTRL_JXX*(2*CTRL_TMC - Ts))/temp_a0
    };
    filter_init(controller->filter_feedforw_x,a,b,Ts);
    
    //y-Axis
    b[0] = (2*CTRL_JYY*(2*CTRL_TMC + Ts))/temp_a0;
    b[1] = -(8*CTRL_JYY*CTRL_TMC)/temp_a0;
    b[2] = (2*CTRL_JYY*(2*CTRL_TMC - Ts))/temp_a0;
    filter_init(controller->filter_feedforw_y,a,b,Ts);
    
    //z-Axis
    b[0] = (2*CTRzL_JZZ*(2*CTRL_TMC + Ts))/temp_a0;
    b[1] = -(8*CTRL_JZZ*CTRL_TMC)/temp_a0;
    b[2] = (2*CTRL_JZZ*(2*CTRL_TMC - Ts))/temp_a0;
    filter_init(controller->filter_feedforw_z,a,b,Ts);
}*/

void ctrl_feed_forward_init_PIID(PIIDController *controller, const float Ts)
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

int  piid_controller_init(PIIDController *controller, const float Ts)
{
    controller->Ts = Ts;
    controller->int_enable = 1;
    controller->int_err1        = (Adams4_f* )calloc(1,sizeof(Adams4_f));
    controller->int_err2        = (Adams4_f* )calloc(1,sizeof(Adams4_f));
    controller->filter_lp_err   = (Filter2nd* )calloc(1,sizeof(Filter2nd));
    controller->filter_hp_err   = (Filter2nd* )calloc(1,sizeof(Filter2nd));
    
    controller->filter_feedforw_x = (Filter2ndFull* )calloc(1,sizeof(Filter2ndFull));
    controller->filter_feedforw_y = (Filter2ndFull* )calloc(1,sizeof(Filter2ndFull));
    controller->filter_feedforw_z = (Filter2ndFull* )calloc(1,sizeof(Filter2ndFull));
    
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
    
    ctrl_feed_forward_init_PIID(controller, Ts);
        
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

void piid_controller_term(PIIDController *controller)
{
    adams4_term(controller->int_err1);
    adams4_term(controller->int_err2);

    free(controller->xi_err);
    free(controller->f_local);
}

void piid_controller_run(PIIDController *controller, float *gyro_input, float *rc_input, float *u_ctrl)
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
    
    //Filter    
    filter_hp_run(controller->filter_hp_err, error, derror);
    filter_lp_run(controller->filter_lp_err, error, error);
    
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
    
    //controller->test_out[0] = controller->xii_err[0];
    //controller->test_out[1] = controller->xii_err[1];
    //controller->test_out[2] = controller->xii_err[2];
    
    /* eval controller */            
    controller->f_local[1] = PIID_KP * error[0] + PIID_KI * controller->xi_err[0] + PIID_KII * controller->xii_err[0] + PIID_KD * derror[0];  
    controller->f_local[2] = PIID_KP * error[1] + PIID_KI * controller->xi_err[1] + PIID_KII * controller->xii_err[1] + PIID_KD * derror[1];
    controller->f_local[3] = PIID_Y_KP * error[2] +  PIID_Y_KI * controller->xi_err[2] + PIID_Y_KII * controller->xii_err[2] +  PIID_Y_KD * derror[2];
    
    /* compensation of NL */
    // controller->f_local[1] -= (CTRL_JYY-CTRL_JZZ)/CTRL_JXX*gyro_input[1]*gyro_input[2];
    // controller->f_local[2] -= (CTRL_JZZ-CTRL_JXX)/CTRL_JYY*gyro_input[0]*gyro_input[2];
    //
    u_ctrl[0] = controller->f_local[1];
    u_ctrl[1] = controller->f_local[2];
    u_ctrl[2] = controller->f_local[3];
    
     
    /* feed forward calculation */
    ctrl_feed_forward_PIID(controller, rc_input);
}


/******************************************
 *NEW Controller
 */

int  new_controller_init(ControllerNew *controller, const float Ts)
{
    controller->Ts = Ts;
    controller->int_enable = 1;
    controller->int_err1        = (Adams4_f* )calloc(1,sizeof(Adams4_f));
    controller->int_err2        = (Adams4_f* )calloc(1,sizeof(Adams4_f));
    controller->filter_lp_err   = (Filter2nd* )calloc(1,sizeof(Filter2nd));
    controller->filter_hp_err   = (Filter2nd* )calloc(1,sizeof(Filter2nd));
    controller->filter_hpd_err  = (Filter2nd* )calloc(1,sizeof(Filter2nd));
    
    controller->filter_feedforw_x = (Filter2ndFull* )calloc(1,sizeof(Filter2ndFull));
    controller->filter_feedforw_y = (Filter2ndFull* )calloc(1,sizeof(Filter2ndFull));
    controller->filter_feedforw_z = (Filter2ndFull* )calloc(1,sizeof(Filter2ndFull));
    
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
    filter_hpd_init(controller->filter_hpd_err , FILT_FG, FILT_D, Ts, 3);
    
    ctrl_feed_forward_init_new(controller, Ts);
        
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

void new_controller_run(ControllerNew *controller, const float *gyro_input, const float *rc_input)
{
    float error[3];
    float derror[3];
    float dderror[3];
    
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
    
    controller->test_out[0] = error[0];
    controller->test_out[1] = error[1];
    controller->test_out[2] = error[2];
    
    //Filter    
    filter_hp_run(controller->filter_hp_err, error, derror);
    filter_hpd_run(controller->filter_hpd_err, error, dderror);
    filter_lp_run(controller->filter_lp_err, error, error);

    
    
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
    controller->f_local[1] = NEW_KP * error[0] + NEW_KI * controller->xi_err[0] + NEW_KII * controller->xii_err[0] + NEW_KD * derror[0] + NEW_KDD * dderror[0];  
    controller->f_local[2] = NEW_KP * error[1] + NEW_KI * controller->xi_err[1] + NEW_KII * controller->xii_err[1] + NEW_KD * derror[1] + NEW_KDD * dderror[1];
    controller->f_local[3] =  PID_KP * error[2] +  PID_KI * controller->xi_err[2] +     0.0f * controller->xii_err[2] +  PID_KD * derror[2];
    
    /* compensation of NL */
    // controller->f_local[1] -= (CTRL_JYY-CTRL_JZZ)/CTRL_JXX*gyro_input[1]*gyro_input[2];
    // controller->f_local[2] -= (CTRL_JZZ-CTRL_JXX)/CTRL_JYY*gyro_input[0]*gyro_input[2];
    
     
    /* feed forward calculation */
    ctrl_feed_forward_new(controller, rc_input);
}

void ctrl_feed_forward_new(ControllerNew *controller, const float *rc_input)
{
    controller->f_local[1] += filter_run(controller->filter_feedforw_x, rc_input[0]);
    controller->f_local[2] += filter_run(controller->filter_feedforw_y, rc_input[1]);
    controller->f_local[3] += filter_run(controller->filter_feedforw_z, rc_input[2]);
}

void ctrl_feed_forward_init_new(ControllerNew *controller, const float Ts)
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
