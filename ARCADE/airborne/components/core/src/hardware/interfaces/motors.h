/*
 * motors.h
 *
 * Created on: 11.06.2010
 * Author: tobi
 */


#ifndef __MOTORS_H__
#define __MOTORS_H__


int motors_init(void);

int motors_write(float pitch, float roll, float yaw, float gas);


#endif /* __MOTORS_H__ */

