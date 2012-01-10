/*
 * motors.c
 *
 * Created on: 11.06.2010
 * Author: tobi
 */


#include <math.h>

#include "motors.h"
#include "util.h"


#define MOTORS_FC


#ifdef MOTORS_FC
#include "../holger_fc/holger_fc.h"
#endif


static int _motors_write(mixer_in_t *data)
{
#ifdef MOTORS_FC
   return fc_write_motors(data);
#endif
}


int motors_init(void)
{
#ifdef MOTORS_FC
   return fc_init();
#endif
}


int motors_write(mixer_in_t *data)
{
   data->pitch = symmetric_limit(data->pitch, 1.0f);
   data->roll = symmetric_limit(data->roll, 1.0f);
   data->yaw = symmetric_limit(data->yaw, 1.0f);
   data->gas = limit(data->gas, 0.0f, 1.0f);
   return _motors_write(data);
}

