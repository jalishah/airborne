

#include "pid.h"
#include "../util/util.h"


void pid_init(pid_controller_t *controller, threadsafe_float_t *p, threadsafe_float_t *i, threadsafe_float_t *d, threadsafe_float_t *max_sum_error)
{
   ASSERT_NOT_NULL(p);
   controller->p = p;
   controller->i = i; /* might be NULL .. */
   if (i != NULL)
   {
      ASSERT_NOT_NULL(max_sum_error);
      controller->max_sum_error = max_sum_error;
      threadsafe_float_init(&controller->sum_error, 0.0f);
   }
   controller->d = d; /* ... this one as well */
   threadsafe_float_init(&controller->prev_error, 0.0f);
}


float pid_control(pid_controller_t *controller, float error, float dt)
{
   float val = threadsafe_float_get(controller->p) * error;
   if (controller->i != NULL)
   {
      float sum_error = threadsafe_float_get(&controller->sum_error);
      sum_error = symmetric_limit(sum_error + error, threadsafe_float_get(controller->max_sum_error));
      val += threadsafe_float_get(controller->i) * dt * sum_error;
      threadsafe_float_set(&controller->sum_error, sum_error);
   }
   if ((controller->d != NULL) && (dt != 0.0f))
   {
      val += threadsafe_float_get(controller->d) * (error - threadsafe_float_get(&controller->prev_error)) / dt;
   }
   threadsafe_float_set(&controller->prev_error, error);
   return val;
}


void pid_reset(pid_controller_t *controller)
{
   threadsafe_float_set(&controller->prev_error, 0.0f);
   threadsafe_float_set(&controller->sum_error, 0.0f);
}

