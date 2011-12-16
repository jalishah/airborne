
/**
 * file: mixer.h
 * author: Tobias Simon (Ilmenau University of Technology)
 */


#ifndef __MIXER_H__
#define __MIXER_H__


#include <stddef.h>


typedef union
{
   struct
   {
      float gas;
      float pitch;
      float roll;
      float yaw;
   };
   float v[4];
}
mixer_in_t;


struct mixer;
typedef struct mixer mixer_t;


/*
 * "init" array layout:
 *
 * m_1: g_0, p_0, r_0, y_0
 *      [...]
 * m_n: g_n, p_n, r_n, y_n
 *
 * g = gas, p = pitch, r = roll, y = yaw, n = #motors
 */
mixer_t *mixer_create(size_t motors, float *init);


/*
 * "out" array layout:
 *
 * v_1, ... ,v_n
 * v_x = motor setpoint [0..1]
 * n = #motors
 */
void mixer_calc(mixer_t *context, float *out, const mixer_in_t *in);


#endif /* __MIXER_H__ */

