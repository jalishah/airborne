#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__

#include "filter.h"
#include "multistep_int.h"
#include "control_paramter.h"


/* type definitions */

/*typedef struct 
{
    float Ts;
    
    float *f_local;
    float *ang_obsv;
    float *ang_rc;
    float *xi_ctrl;
    float *omg_d;
    
    Adams4_f *int_obsv;
    Adams4_f *int_rc;
    Adams4_f *int_ctrl;
}ControllerData;*/

typedef struct 
{
    float Ts;
    
    float *f_local;
    float *xi_err;
    
    Adams4_f *int_err;
    
    Filter2nd     *filter_lp_err;
    Filter2nd     *filter_hp_err;
    Filter2ndFull *filter_feedforw_x;
    Filter2ndFull *filter_feedforw_y;
    Filter2ndFull *filter_feedforw_z;
    
    float ringbuf[CTRL_NUM_TSTEP][3];
    int ringbuf_idx;
    
    float test_out[3];
}PIDController;

typedef struct 
{
    float Ts;
    
    float *f_local;
    float *xi_err;
    float *xii_err;
    
    Adams4_f *int_err1;
    Adams4_f *int_err2;
    
    unsigned int int_enable;
    
    Filter2nd     *filter_lp_err;
    Filter2nd     *filter_hp_err;
    Filter2ndFull *filter_feedforw_x;
    Filter2ndFull *filter_feedforw_y;
    Filter2ndFull *filter_feedforw_z;
    
    float ringbuf[3*CTRL_NUM_TSTEP];
    int ringbuf_idx;
    
    float test_out[3];
}PIIDController;

typedef struct
{
    float Ts;
    
    float *f_local;
    float *xi_err;
    float *xii_err;
    
    Adams4_f *int_err1;
    Adams4_f *int_err2;
    
    unsigned int int_enable;
    
    Filter2nd     *filter_lp_err;
    Filter2nd     *filter_hp_err;
    Filter2nd     *filter_hpd_err;
    Filter2ndFull *filter_feedforw_x;
    Filter2ndFull *filter_feedforw_y;
    Filter2ndFull *filter_feedforw_z;
    
    float ringbuf[3*CTRL_NUM_TSTEP];
    int ringbuf_idx;
    
    float test_out[3];
}ControllerNew;

/*void controller_run(ControllerData *controller, const float *gyro_input, const float *rc_input);
int  controller_init(ControllerData *controller, float sample_time);
void controller_term(ControllerData *controller);*/

void pid_controller_run(PIDController *controller, const float *gyro_input, const float *rc_input);
int  pid_controller_init(PIDController *controller, const float sample_time);
void pid_controller_term(PIDController *controller);

void piid_controller_run(PIIDController *controller, float *gyro_input, float *rc_input, float *u_ctrl);
int  piid_controller_init(PIIDController *controller, const float sample_time);
void piid_controller_term(PIIDController *controller);

void new_controller_run(ControllerNew *controller, const float *gyro_input, const float *rc_input);
int  new_controller_init(ControllerNew *controller, const float sample_time);

/*void ctrl_feed_forward(PIDController *controller, const float *rc_input);
void ctrl_feed_forward_init(PIDController *controller, const float Ts);*/

void ctrl_feed_forward_PIID(PIIDController *controller, const float *rc_input);
void ctrl_feed_forward_init_PIID(PIIDController *controller, const float Ts);

void ctrl_feed_forward_new(ControllerNew *controller, const float *rc_input);
void ctrl_feed_forward_init_new(ControllerNew *controller, const float Ts);


/* interal functions */
/*void outer_controller(ControllerData *controller);
void inner_controller(ControllerData *controller, const float *rc_input, const float *gyro_input);
void pi_controller(ControllerData *controller,const float *rc_input, const float *gyro_input);*/
/* observer */
/*void observer_run(ControllerData *controller, const float *gyro_input);*/

#endif /* __CONTROLLER_H__ */
