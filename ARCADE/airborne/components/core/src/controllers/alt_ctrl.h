
/*
 * alt_ctrl.h
 *
 * Created on: 26.09.2010
 * Author: tobi
 */


#ifndef __ALT_CTRL_H__
#define __ALT_CTRL_H__


void alt_ctrl_init(void);

void alt_setpoint_set(float yaw);

float alt_setpoint_get(void);

float alt_ctrl_step(float alt, float alt_speed, float dt);

void alt_ctrl_reset(void);


#endif /* __ALT_CTRL_H__ */

