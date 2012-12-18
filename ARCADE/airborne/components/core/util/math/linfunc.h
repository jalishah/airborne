

#ifndef __LINFUNC_H__
#define __LINFUNC_H__


#include "vec2.h"


typedef struct
{
   float m;
   float n;
}
linfunc_t;


void linfunc_init_points(linfunc_t *func, vec2_t *v1, vec2_t *v2);


float linfunc_calc(linfunc_t *func, float x);


#endif /* __LINFUNC_H__ */

