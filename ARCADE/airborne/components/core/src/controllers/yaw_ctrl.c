
/*
 * yaw_ctrl.c
 *
 *  Created on: 26.09.2010
 *      Author: tobi
 */


#include <malloc.h>
#include <math.h>

#include <util.h>
#include <opcd_params.h>
#include <threadsafe_types.h>

#include "yaw_ctrl.h"
#include "pid.h"
#include "../util/logger/logger.h"


static pid_controller_t controller;

/* setpoints: */
static tsfloat_t pos; /* yaw position */
static tsfloat_t speed; /* yaw speed */

/* configurable parameters: */
static tsfloat_t speed_slope;
static tsfloat_t speed_min;
static tsfloat_t speed_std;
static tsfloat_t speed_max;
static tsfloat_t speed_p;
static tsfloat_t speed_i;
static tsfloat_t speed_imax;
static tsint_t manual;




static float circle_err(float pos, float dest)
{
   float err;
   if ((dest < pos) && (pos - dest < M_PI))
   {
      err = pos - dest;
   }
   else if ((dest < pos) && (pos - dest >= M_PI))
   {
      err = -(2.0f * (float)M_PI - (pos - dest));
   }
   else if ((dest > pos) && (dest - pos < M_PI))
   {
      err = -(dest - pos);
   }
   else if ((dest > pos) && (dest - pos >= M_PI))
   {
      err = 2.0f * (float)M_PI - (dest - pos);
   }
   else
   {
      err = 0.0f;
   }
   return err;
}


static float speed_func(float angle)
{
   float _speed = tsfloat_get(&speed);
   return sym_limit(tsfloat_get(&speed_slope) * angle, _speed);
}


void yaw_ctrl_init(void)
{
   ASSERT_ONCE();
   opcd_param_t params[] =
   {
      {"speed_min", &speed_min},
      {"speed_std", &speed_std},
      {"speed_max", &speed_max},
      {"speed_slope", &speed_slope},
      {"speed_p", &speed_p.value},
      {"speed_i", &speed_i.value},
      {"speed_imax", &speed_imax.value},
      {"manual", &manual},
      OPCD_PARAMS_END
   };
   opcd_params_apply("controllers.yaw.", params);

   tsfloat_init(&pos, 0.0f);
   tsfloat_init(&speed, tsfloat_get(&speed_std));

   pid_init(&controller, &speed_p, &speed_i, NULL, &speed_imax);
}


int yaw_ctrl_set_pos(float _pos)
{
   if ((_pos < 0) || (_pos > 2.0 * M_PI))
   {
      LOG(LL_ERROR, "invalid yaw setpoint: %f, out of bounds: (0.0, 2.0 * M_PI)", _pos);
      return -1;
   }
   tsfloat_set(&pos, _pos);
   return 0;
}


float yaw_ctrl_get_pos(void)
{
   return tsfloat_get(&pos);
}


int yaw_ctrl_set_speed(float _speed)
{
   float _speed_min = tsfloat_get(&speed_min);
   float _speed_max = tsfloat_get(&speed_max);
   if ((_speed < _speed_min) || (_speed > _speed_max))
   {
      LOG(LL_ERROR, "invalid yaw speed: %f, out of bounds: (%f, %f)",
          _speed, _speed_min, _speed_max);
      return -1;
   }
   tsfloat_set(&speed, _speed);
   return 0;
}


float yaw_ctrl_get_speed(void)
{
   return tsfloat_get(&speed);
}


void yaw_ctrl_std_speed(void)
{
   tsfloat_set(&speed, tsfloat_get(&speed_std));
}


float yaw_ctrl_step(float *err_out, float yaw, float _speed, float dt)
{
   float err;
   float yaw_ctrl;
   if (tsint_get(&manual))
   {
      yaw_ctrl = 0.0f;
      err = 0.0; /* we control nothing, so the error is always 0 */
   }
   else
   {
      err = circle_err(yaw, tsfloat_get(&pos));
      float speed_setpoint = -speed_func(err);
      float speed_err = speed_setpoint - _speed;
      yaw_ctrl = pid_control(&controller, speed_err, dt);
   }
   *err_out = err;
   return yaw_ctrl;
}


void yaw_ctrl_reset(void)
{
   pid_reset(&controller);
}

