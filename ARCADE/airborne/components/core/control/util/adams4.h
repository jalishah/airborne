
#ifndef __ADAMS4_H__
#define __ADAMS4_H__


#include <stdbool.h>
#include <stddef.h>


typedef struct 
{
   size_t dim;
   float *f0;
   float *f1;
   float *f2;
   float *f3;
}
adams4_t;


void adams4_init(adams4_t *a, const unsigned int dim);

void adams4_run(adams4_t *a, float *x, float ts, bool enabled);

void adams4_term(adams4_t *a);


#endif /* __ADAMS4_H__ */

