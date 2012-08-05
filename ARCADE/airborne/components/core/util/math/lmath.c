
/*
 * local math library
 *
 * author: tobi
 */

#include <math.h>

#include "lmath.h"

float normalize_euler_0_2pi(float euler_angle)
{
   if (euler_angle < 0)
   {
      euler_angle += (float)(2 * M_PI);
   }
   return euler_angle;
}


float deg2rad(float x)
{
   return x * (float)(M_PI / 180.0);
}


float rad2deg(float x)
{
   return x * (float)(180.0 / M_PI);
}


