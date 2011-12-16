
/*
 * alt_ctrl.c
 *
 * Created on: 26.09.2010
 * Author: tobi
 */


#include "alt_ctrl.h"
#include "pid.h"
#include "../util/util.h"
#include "../util/config/config.h"
#include "../interfaces/params.h"


static pid_controller_t controller;
static threadsafe_float_t speed_p;
static threadsafe_float_t speed_i;
static threadsafe_float_t speed_imax;
static threadsafe_float_t setpoint;
static int manual;


static config_t options[] =
{
   {"speed_p", &speed_p.value},
   {"speed_i", &speed_i.value},
   {"speed_imax", &speed_imax.value},
   {"manual", &manual},
   {NULL, NULL}
};


static float speed_func(float dist)
{
   return symmetric_limit(dist, 1.0f) * 0.6;
}


void alt_setpoint_set(float yaw)
{
   threadsafe_float_set(&setpoint, yaw);
}


float alt_setpoint_get(void)
{
   return threadsafe_float_get(&setpoint);
}


float alt_ctrl_step(float alt, float speed, float dt)
{
   float alt_err = threadsafe_float_get(&setpoint) - alt;
   float spd_sp = speed_func(alt_err);
   float spd_err = spd_sp - speed;
   float gas;
   if (manual)
   {
      gas = 1.0f;
   }
   else
   {
      gas = 0.5f + pid_control(&controller, spd_err, dt);
   }
   return gas;
}


void alt_ctrl_init(void)
{
   ASSERT_ONCE();

   threadsafe_float_init(&speed_p, 0.0);
   threadsafe_float_init(&speed_i, 0.0);
   threadsafe_float_init(&speed_imax, 0.0);
   param_add("alt_speed_p", &speed_p);
   param_add("alt_speed_i", &speed_i);
   param_add("alt_speed_imax", &speed_imax);

   config_apply("alt_ctrl", options);

   threadsafe_float_init(&setpoint, 0.0f);

   pid_init(&controller, &speed_p, &speed_i, NULL, &speed_imax);
}


void alt_ctrl_reset(void)
{
   pid_reset(&controller);
}

