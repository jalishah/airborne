

#ifndef PID_H
#define PID_H


#include <threadsafe_types.h>


typedef struct
{
   threadsafe_float_t *p;
   threadsafe_float_t *i;
   threadsafe_float_t *d;
   threadsafe_float_t *max_sum_error;
   threadsafe_float_t prev_error;
   threadsafe_float_t sum_error;
}
pid_controller_t;


void pid_init(pid_controller_t *controller, threadsafe_float_t *p, threadsafe_float_t *i, threadsafe_float_t *d, threadsafe_float_t *max_sum_error);

float pid_control(pid_controller_t *controller, float error, float dt);

void pid_reset(pid_controller_t *controller);


#endif /* PID_H */

