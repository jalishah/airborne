
/*
 * deadzone.h
 * purpose: implementation of deadzone calculations for remote control sticks
 */


#include <math.h>

#include "deadzone.h"



void deadzone_init(deadzone_t *dz, float xmin, float xmax, float ymax)
{
   dz->x0 = xmin;
   dz->ymax = ymax;
   dz->a = ymax / (xmax - xmin);
}


float deadzone_calc(deadzone_t *dz, float x)
{
   float y;
   float sign = (x >= 0) ? 1.0f : -1.0f;
   x = fabs(x);
   if (x <= dz->x0)
   {
      y = 0.0f;
   }
   else
   {
      y = fmin(dz->ymax, dz->a * (x - dz->x0));
   }
   return y * sign;
}

