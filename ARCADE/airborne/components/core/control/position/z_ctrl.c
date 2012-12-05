
/*
 * File: z_ctrl.c
 * Type: PID controller
 * Purpose: altitude control using ground or MSL sensor input
 * Design Pattern: Singleton using z_ctrl_init
 *
 * Responsibilities:
 *   - self-configuration through OPCD
 *   - setpoint management
 *   - mode management (ground vs. MSL altitude)
 *   - controller state management
 *
 * Author: Tobias Simon, Ilmenau University of Technology
 */


#include <util.h>
#include <opcd_params.h>
#include <threadsafe_types.h>

#include "z_ctrl.h"
#include "../util/pid.h"


static pid_controller_t controller;


/* initialized by "init" parameter: */
static float z_neutral_gas;

/* following thread-safe variables are initialized by pid_init: */
static tsfloat_t speed_p;
static tsfloat_t speed_i;
static tsfloat_t speed_imax;

/* following thread-safe variables are initialized by OPCD: */
static tsfloat_t init_setpoint;
static tsint_t init_mode_is_ground;

/* following thread-safe variables are initialized by z_ctrl_init: */
static tsfloat_t setpoint;
static tsint_t mode_is_ground;



/* returns the desired vertical speed given a error */
static float speed_func(float err)
{
   return sym_limit(err, 1.0f) * 0.6;
}


void z_ctrl_set_setpoint(z_setpoint_t z_setpoint)
{
   tsfloat_set(&setpoint, z_setpoint.val);
   tsint_set(&mode_is_ground, z_setpoint.mode_is_ground);
}


float z_ctrl_get_setpoint(void)
{
   return tsfloat_get(&setpoint);
}


int z_ctrl_mode_is_ground(void)
{
   return tsint_get(&mode_is_ground);
}


/*
 * perform controller step
 * parameters ground_z_pos, z_pos, float speed: need to be filtered (e.g. kalman)
 * returns vertical controller output
 */
float z_ctrl_step(float *z_err, float ground_z_pos, float z_pos, float speed, float dt)
{   
   float _z_err;
   if (tsint_get(&mode_is_ground))
   {
      _z_err = tsfloat_get(&setpoint) - ground_z_pos;
   }
   else
   {
      _z_err = tsfloat_get(&setpoint) - z_pos;
   }
   float spd_sp = speed_func(_z_err);
   float spd_err = spd_sp - speed;
   float val;
   val = z_neutral_gas + pid_control(&controller, /*spd_err*/ _z_err, speed, dt);
   *z_err = _z_err;
   return val;
}


void z_ctrl_init(float neutral_gas)
{
   ASSERT_ONCE();
   
   opcd_param_t params[] =
   {
      {"speed_p", &speed_p},
      {"speed_i", &speed_i},
      {"speed_imax", &speed_imax},
      {"init_setpoint", &init_setpoint},
      {"init_mode_is_ground", &init_mode_is_ground},
      OPCD_PARAMS_END
   };
   opcd_params_apply("controllers.altitude.", params);

   tsfloat_init(&setpoint, tsfloat_get(&init_setpoint));
   tsint_init(&mode_is_ground, tsint_get(&init_mode_is_ground));
   pid_init(&controller, &speed_p, &speed_i, NULL, &speed_imax);
   z_neutral_gas = neutral_gas;
}


void z_ctrl_reset(void)
{
   pid_reset(&controller);
}

