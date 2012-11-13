
/*
   inverse motor coupling matrix - interface

   Copyright (C) 2012 Tobias Simon, Ilmenau University of Technology

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
 */


#ifndef __INV_COUPLING_H__
#define __INV_COUPLING_H__


#include <meschach/matrix.h>


typedef struct coupling
{
   size_t n_motors;
   MAT *matrix;
   VEC *in;
   VEC *out;
}
inv_coupling_t;


/*
 * "init" array layout:
 *
 * m_1: g_0, p_0, r_0, y_0
 * [...]
 * m_n: g_n, p_n, r_n, y_n
 *
 * g = gas, p = pitch, r = roll, y = yaw, n = #motors
 */
void inv_coupling_init(inv_coupling_t *inv_coupling, const size_t motors, const float *init);


/*
 * "out" array layout:
 *
 * v_1, ... ,v_n
 * v_x = motor setpoint [0..1]
 * n = #motors
 */
void inv_coupling_calc(const inv_coupling_t *inv_coupling, float *out, const float *in);


#endif /* __INV_COUPLING__ */

