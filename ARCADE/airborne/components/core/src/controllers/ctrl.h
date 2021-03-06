
/*
 * File: ctrl.c
 * Type: set of PID controllers
 * Purpose: runs multiple controllers (x,y,z,yaw) and executes attitude control
 * Design Pattern: Singleton using ctrl_init
 *
 * Responsibilities:
 *   - self-configuration through OPCD
 *   - sub-controller management
 *   - controller state management
 *
 * Author: Tobias Simon, Ilmenau University of Technology
 */


#ifndef __CTRL_H__
#define __CTRL_H__

#include <threadsafe_types.h>
#include <core.pb-c.h>

#include "../model/model.h"
#include "../sensor_actor/interfaces/motors.h"


typedef struct
{
   tsfloat_t yaw_error;
   tsfloat_t alt_error;
   tsfloat_t x_error;
   tsfloat_t y_error;
}
controller_errors_t;


void ctrl_init(void);

void ctrl_override(float pitch, float roll, float yaw, float gas);

void ctrl_stop_override(void);
 

/*
 * reset controllers (affects i-parts)
 */
void ctrl_reset(void);


/*
 * run the controllers
 */
void ctrl_step(mixer_in_t *data, float dt, model_state_t *model_state);


/*
 * sets the setpoint for type to val
 */
int ctrl_set_data(CtrlParam param, float data);


#endif /* __CTRL_H__ */

