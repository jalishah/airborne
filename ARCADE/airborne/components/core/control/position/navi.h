
#ifndef __NAVI_H__
#define __NAVI_H__


#include "../../util/math/vec2.h"


/*
 * allocates and initializes memory for navigation control subsystem
 */
void navi_init(void);


/*
 * resets i-part(s) of the navigation algorithm
 */
void navi_reset(void);


/*
 * setter for x position
 */
void navi_set_dest_x(float x);


/*
 * setter for y position
 */
void navi_set_dest_y(float y);


/*
 * getter for x position
 */
float navi_get_dest_x(void);


/*
 * getter for y position
 */
float navi_get_dest_y(void);


/*
 * set travel speed to standard
 */
void navi_reset_travel_speed(void);


/*
 * set navi travel speed
 */
int navi_set_travel_speed(float speed);


/*
 * executes navigation control subsystem
 */
void navi_run(vec2_t *speed_setpoint, vec2_t *pos, float dt);


#endif /* __NAVI_H__ */

