
/*
   ARCADE Motors State Tracking

   Copyright (C) 2012 Tobias Simon, Ilmenau University of Technology

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
 */


#ifndef __MOTORS_STATE_H__
#define __MOTORS_STATE_H__


#include "flight_state.h"


/* initializes motors state tracker */
void motors_state_init(float _gas_start, float _gas_stop);


/* indicates if the motors are safe */
int motors_state_safe(void);


/* indicates if the motors are spinning */
int motors_state_spinning(void);


/* indicates if the controller inputs are used  */
int motors_state_controllable(void);


/* updates the motor state machine */
void motors_state_update(flight_state_t flight_state, int lock, float gas, float dt, int start_allowed);


#endif /* __MOTORS_STATE_H__ */

