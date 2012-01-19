

#include "pid.h"
#include "../util/util.h"


void pid_init(pid_controller_t *controller, tsfloat_t *p, tsfloat_t *i, tsfloat_t *d, tsfloat_t *max_sum_error)
{
   ASSERT_NOT_NULL(p);
   controller->p = p;
   controller->i = i; /* might be NULL .. */
   if (i != NULL)
   {
      ASSERT_NOT_NULL(max_sum_error);
      controller->max_sum_error = max_sum_error;
      tsfloat_init(&controller->sum_error, 0.0f);
   }
   controller->d = d; /* ... this one as well */
   tsfloat_init(&controller->prev_error, 0.0f);
}


float pid_control(pid_controller_t *controller, float error, float dt)
{
   float val = tsfloat_get(controller->p) * error;
   if (controller->i != NULL)
   {
      float sum_error = tsfloat_get(&controller->sum_error);
      sum_error = sym_limit(sum_error + error, tsfloat_get(controller->max_sum_error));
      val += tsfloat_get(controller->i) * dt * sum_error;
      tsfloat_set(&controller->sum_error, sum_error);
   }
   if ((controller->d != NULL) && (dt != 0.0f))
   {
      val += tsfloat_get(controller->d) * (error - tsfloat_get(&controller->prev_error)) / dt;
   }
   tsfloat_set(&controller->prev_error, error);
   return val;
}


void pid_reset(pid_controller_t *controller)
{
   tsfloat_set(&controller->prev_error, 0.0f);
   tsfloat_set(&controller->sum_error, 0.0f);
}

