

#include "pid.h"
#include "../util/util.h"


void pid_init(pid_controller_t *controller, threadsafe_float_t *p, threadsafe_float_t *i, threadsafe_float_t *d, threadsafe_float_t *max_sum_error)
{
   ASSERT_NOT_NULL(p);
   controller->p = p;
   controller->i = i; /* might be NULL .. */
   controller->d = d; /* ... this one as well */
   controller->max_sum_error = max_sum_error;
   pid_reset(controller);
}


float pid_control(pid_controller_t *controller, float error, float dt)
{
   float val;
   controller->sum_error = symmetric_limit(controller->sum_error + error, threadsafe_float_get(controller->max_sum_error));
   val = threadsafe_float_get(controller->p) * error;
   if (controller->i != NULL)
   {
      val += threadsafe_float_get(controller->i) * dt * controller->sum_error;
   }
   if (controller->d != NULL)
   {
      val += threadsafe_float_get(controller->d) * (error - controller->prev_error) / dt;
   }
   controller->prev_error = error;
   return val;
}


void pid_reset(pid_controller_t *controller)
{
   controller->prev_error = 0;
   controller->sum_error = 0;
}
