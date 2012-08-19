
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
coupling_t *coupling_create(const unsigned int motors, const float *init);


/*
 * "out" array layout:
 *
 * v_1, ... ,v_n
 * v_x = motor setpoint [0..1]
 * n = #motors
 */
void coupling_calc(const coupling_t *coupling, float *out, const float *in);


/*
 * returns number of motors in coupling
 */
unsigned int coupling_motors(const coupling_t *coupling);


#endif /* __COUPLING__ */

