
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


void att_ctrl_step(vec2_t *ctrl, vec2_t *err, const float dt, const vec2_t *pos, const vec2_t *speed, const vec2_t *setp)
{
   FOR_EACH(i, controllers)
   {
      float error = setp->vec[i] + tsfloat_get(&bias[i]) - pos->vec[i];
      err->vec[i] = error;
      ctrl->vec[i] = pid_control(&controllers[i], error, speed->vec[i], dt);
   }
}

