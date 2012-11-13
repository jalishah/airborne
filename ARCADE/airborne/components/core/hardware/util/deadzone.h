
/*
 * deadzone.h
 * purpose: interface to deadzone calculations for remote control sticks
 */


#ifndef DEADZONE_H
#define DEADZONE_H


#include <math.h>


typedef struct
{
   float x0;
   float a;
   float ymax;
}
deadzone_t;


void deadzone_init(deadzone_t *dz, float xmin, float xmax, float ymax);

float deadzone_calc(deadzone_t *dz, float x);


#endif /* DEADZONE_H */

