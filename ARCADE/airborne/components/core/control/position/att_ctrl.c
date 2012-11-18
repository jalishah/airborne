
/*
 * File: att_ctrl.c
 * Purpose: attitude controller
 * Author: Tobias Simon, Ilmenau University of Technology
 */


#include <stdint.h>
#include <opcd_params.h>
#include <threadsafe_types.h>
#include <util.h>

#include "att_ctrl.h"
#include "../util/pid.h"


static tsfloat_t angle_p;
static tsfloat_t angle_i;
static tsfloat_t angle_i_max;
static tsfloat_t angle_d;
static tsfloat_t bias[2];
static tsfloat_t angle_max;

static pid_controller_t controllers[2];


void att_ctrl_init(void)
{
   ASSERT_ONCE();

   /* load parameters: */
   opcd_param_t params[] =
   {
      {"p", &angle_p.value},
      {"i", &angle_i.value},
      {"i_max", &angle_i_max.value},
      {"d", &angle_d.value},
      {"pitch_bias", &bias[0]},
      {"roll_bias", &bias[1]},
      {"angle_max", &angle_max},
      OPCD_PARAMS_END
   };
   opcd_params_apply("controllers.attitude.", params);
   
   /* initialize controllers: */
   FOR_EACH(i, controllers)
   {
      pid_init(&controllers[i], &angle_p, &angle_i, &angle_d, &angle_i_max);
   }
}


void att_ctrl_reset(void)
{
   FOR_EACH(i, controllers)
   {
      pid_reset(&controllers[i]);
   }
}


void att_ctrl_step(vec2_t *ctrl, const float dt, const vec2_t *pos, const vec2_t *setp)
{
   float _angle_max = tsfloat_get(&angle_max);
   FOR_EACH(i, controllers)
   {
      /* limit setpoint: */
      float lim_sp = sym_limit(setp->vec[i], _angle_max);
      /* run controller: */
      ctrl->vec[i] = pid_control(&controllers[i], lim_sp + tsfloat_get(&bias[i]) - pos->vec[i], dt);
   }
}

