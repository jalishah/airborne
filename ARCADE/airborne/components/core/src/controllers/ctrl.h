
/*
 * ctrl.h
 *
 * Created on: 26.09.2010
 * Author: tobi
 */


#ifndef __CTRL_H__
#define __CTRL_H__

#include <threadsafe_types.h>
#include <core.pb-c.h>

#include "../model/model.h"
#include "../sensor_actor/interfaces/motors.h"


typedef struct
{
   threadsafe_float_t yaw_error;
   threadsafe_float_t alt_error;
   threadsafe_float_t x_error;
   threadsafe_float_t y_error;
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
 * sets the setpoint
 */
int ctrl_set_setpoint(CtrlData *data);


/*
 * returns the current error for type
 */
size_t ctrl_get_err(float *out, CtrlType type);


#endif /* __CTRL_H__ */

