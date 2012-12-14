

#include "linfunc.h"


void linfunc_init_points(linfunc_t *func, vec2_t *v1, vec2_t *v2)
{
   func->m = (v2->y - v1->y) / (v2->x - v1->x);
   func->n = v1->y - func->m * v1->x;
}


float linfunc_calc(linfunc_t *func, float x)
{
   return func->m * x + func->n;   
}

