
/*
 * file: coupling.h
 * author: Tobias Simon (Ilmenau University of Technology)
 */


#ifndef __COUPLING_H__
#define __COUPLING_H__


#include <stddef.h>
#include <meschach/matrix.h>


typedef struct coupling
{
   size_t n_motors;
   MAT *matrix;
   VEC *in;
   VEC *out;
}
coupling_t;


/*
 * "init" array layout:
 *
 * m_1: g_0, p_0, r_0, y_0
 * [...]
 * m_n: g_n, p_n, r_n, y_n
 *
 * g = gas, p = pitch, r = roll, y = yaw, n = #motors
 */
void coupling_init(coupling_t *coupling, const size_t motors, const float *init);


/*
 * "out" array layout:
 *
 * v_1, ... ,v_n
 * v_x = motor setpoint [0..1]
 * n = #motors
 */
void coupling_calc(const coupling_t *coupling, float *out, const float *in);


#endif /* __COUPLING__ */

