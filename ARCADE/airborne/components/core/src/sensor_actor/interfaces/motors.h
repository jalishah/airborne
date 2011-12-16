/*
 * motors.h
 *
 * Created on: 11.06.2010
 * Author: tobi
 */


#ifndef __MOTORS_H__
#define __MOTORS_H__


#include "../../platform/mixer.h"


int motors_init(void);

int motors_write(mixer_in_t *data);


#endif /* __MOTORS_H__ */

