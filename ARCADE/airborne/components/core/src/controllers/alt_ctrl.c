
/*
 * alt_ctrl.c
 *
 * Created on: 26.09.2010
 * Author: tobi
 */


#include "alt_ctrl.h"
#include "pid.h"
#include "util.h"
#include "opcd_params.h"
#include "threadsafe_types.h"


static pid_controller_t controller;
static tsfloat_t speed_p;
static tsfloat_t speed_i;
static tsfloat_t speed_imax;
static tsfloat_t setpoint;
static tsint_t manual;


static float speed_func(float dist)
{
   return sym_limit(dist, 1.0f) * 0.6;
}


void alt_setpoint_set(float yaw)
{
   tsfloat_set(&setpoint, yaw);
}


float alt_setpoint_get(void)
{
   return tsfloat_get(&setpoint);
}


float alt_ctrl_step(float alt_err, float speed, float dt)
{
   float spd_sp = speed_func(alt_err);
   float spd_err = 0.2/*spd_sp*/ - speed;
   float gas;
   if (tsint_get(&manual))
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
   opcd_param_t params[] =
   {
      {"speed_p", &speed_p},
      {"speed_i", &speed_i},
      {"speed_imax", &speed_imax},
      {"manual", &manual},
      OPCD_PARAMS_END
   };
   opcd_params_apply("controllers.altitude.", params);

   tsfloat_init(&setpoint, 1.0f);
   pid_init(&controller, &speed_p, &speed_i, NULL, &speed_imax);
}


void alt_ctrl_reset(void)
{
   pid_reset(&controller);
}

