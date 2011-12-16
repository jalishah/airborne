

#ifndef PID_H
#define PID_H


#include "../util/threads/threadsafe_types.h"


typedef struct
{
   threadsafe_float_t *p;
   threadsafe_float_t *i;
   threadsafe_float_t *d;
   threadsafe_float_t *max_sum_error;
   unsigned char flags;
   float prev_error;
   float sum_error;
}
pid_controller_t;


void pid_init(pid_controller_t *controller, threadsafe_float_t *p, threadsafe_float_t *i, threadsafe_float_t *d, threadsafe_float_t *max_sum_error);

float pid_control(pid_controller_t *controller, float error, float dt);

void pid_reset(pid_controller_t *controller);


#endif /* PID_H */

