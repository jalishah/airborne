
/*
   Kalman Filter Interface for System:

   | 1 dt | * | p | + | 0.5 * dt ^ 2 | * | a | = | p |
   | 0  1 | * | v |   |     dt       |   | v |
 
   Copyright (C) 2012 Tobias Simon, Ilmenau University of Technology
   Copyright (C) 2012 Jan Roemisch, Ilmenau University of Technology

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
 */


#ifndef __KALMAN_H__
#define __KALMAN_H__


#include <meschach/matrix.h>


typedef struct
{
   /* configuration and constant matrices: */
   MAT *Q; /* process noise */
   MAT *R; /* measurement noise */
   MAT *I; /* identity matrix */

   /* state and transition vectors/matrices: */
   VEC *x; /* state (location and velocity) */
   VEC *z; /* measurement (location) */
   MAT *A; /* system matrix */
   MAT *B; /* control matrix */
   MAT *P; /* error covariance */
   VEC *u; /* control (acceleration) */
   MAT *H; /* observer matrix */
   MAT *K; /* kalman gain */

   /*  vectors and matrices for calculations: */
   VEC *t0;
   VEC *t1;
   MAT *T0;
   MAT *T1;
}
kalman_t;


typedef struct
{
   float pos;
   float speed;
}
kalman_out_t;


typedef struct
{
   float dt; /* time elapsed since last kalman step */
   float pos; /* position in m */
   float acc; /* acceleration min m/s^2 */
}
kalman_in_t;


typedef struct
{
   float process_var;
   float measurement_var;
}
kalman_config_t;


/*
 * executes kalman predict and correct step
 */
void kalman_run(kalman_out_t *out, kalman_t *kalman, const kalman_in_t *in);


/*
 * initializes a kalman filter
 */
void kalman_init(kalman_t *kf, float q, float r, float pos, float speed);


#endif /* __KALMAN_H__ */

