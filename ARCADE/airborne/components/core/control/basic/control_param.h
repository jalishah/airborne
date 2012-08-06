
#ifndef __SYS_PARAM_H__
#define __SYS_PARAM_H__

/* ----- system parameter ----- */
#define CTRL_L 0.2025f
#define CTRL_VOLTAGE 15.0f	/* voltage for the input compensation (should be measured later) */

#define CTRL_JXX 1*0.0097f
#define CTRL_JYY 1*0.0097f
#define CTRL_JZZ 0.45*1.273177e-002f

#define CTRL_TMC 0.06f


/* ----- parameter for input compensation ----- */
/* rpm^2 =a*voltage^1.5*i2c^b */
#define CTRL_F_A 609.6137f
#define CTRL_F_B 1.3154f

/* F = c*rpm^2 */
#define CTRL_F_C 1.5866e-007f

/* tau = d*rpm^2 */
#define CTRL_F_D 4.0174e-009f



/* ----- control parameter ----- */
#define CTRL_NUM_TSTEP 7

/* outer controller */
#define CTRL_OUT_KP 0.08f
#define CTRL_OUT_KI 0.12f

/* innner controller */
#define CTRL_LAMBDA_X 0.045f
#define CTRL_LAMBDA_Y 0.045f
#define CTRL_LAMBDA_Z 0.045f

#define PID_KI 0.076195f
#define PID_KP 0.049001f
#define PID_KD 0.00025829

#define PIID_KP  0.233410f
#define PIID_KD  0.010079f
#define PIID_KI  1.8017f
#define PIID_KII 4.63590f

#define PIID_Y_KP  0.108f
#define PIID_Y_KD  0.00648f
#define PIID_Y_KI  0.45f
#define PIID_Y_KII 0.0f

#define TEMP_K  18.0f;
#define TEMP_T  0.07f;

/* Filter Parameter */
#define FILT_FG 10.0f
#define FILT_D  0.95f


#endif /* __SYS_PARAM_H__ */
