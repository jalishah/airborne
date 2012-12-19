

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

