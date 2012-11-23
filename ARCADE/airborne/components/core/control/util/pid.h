

#ifndef PID_H
#define PID_H


#include <threadsafe_types.h>


typedef struct
{
   tsfloat_t *p;
   tsfloat_t *i;
   tsfloat_t *d;
   tsfloat_t *max_sum_error;
   tsfloat_t prev_error;
   tsfloat_t sum_error;
}
pid_controller_t;


void pid_init(pid_controller_t *controller, tsfloat_t *p, tsfloat_t *i, tsfloat_t *d, tsfloat_t *max_sum_error);

float pid_control(pid_controller_t *controller, float error, float speed, float dt);

void pid_reset(pid_controller_t *controller);


#endif /* PID_H */

