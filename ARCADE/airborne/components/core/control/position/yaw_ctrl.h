
/*
 * yaw_ctrl.h
 *
 * Created on: 26.09.2011
 * Author: tobi
 */


#ifndef __YAW_CTRL_H__
#define __YAW_CTRL_H__


void yaw_ctrl_init(void);

int yaw_ctrl_set_pos(float pos);

float yaw_ctrl_get_pos(void);

float yaw_ctrl_step(float *err_out, float yaw, float speed, float dt);

void yaw_ctrl_reset(void);


#endif /* __YAW_CTRL_H__ */

