
/*
 * file: coupling.h
 * author: Tobias Simon (Ilmenau University of Technology)
 */


#ifndef __COUPLING_H__
#define __COUPLING_H__


#include <stddef.h>


struct coupling;
typedef struct coupling coupling_t;


/*
 * "init" array layout:
 *
 * m_1: g_0, p_0, r_0, y_0
 * [...]
 * m_n: g_n, p_n, r_n, y_n
 *
 * g = gas, p = pitch, r = roll, y = yaw, n = #motors
 */
coupling_t *coupling_create(size_t motors, float *init);


/*
 * "out" array layout:
 *
 * v_1, ... ,v_n
 * v_x = motor setpoint [0..1]
 * n = #motors
 */
void coupling_calc(coupling_t *coupling, float *out, float *in);


#endif /* __COUPLING__ */

