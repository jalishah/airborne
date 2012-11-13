
/*
   Kalman Filter for System:
 
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


#include <meschach/matrix2.h>

#include <util.h>

#include "kalman.h"


void kalman_init(kalman_t *kf, float q, float r, float pos, float speed)
{
   kf->t0 = v_get(2);
   ASSERT_NOT_NULL(kf->t0);
   kf->t1 = v_get(2);
   ASSERT_NOT_NULL(kf->t1);
   kf->T0 = m_get(2, 2);
   ASSERT_NOT_NULL(kf->T0);
   kf->T1 = m_get(2, 2);
   ASSERT_NOT_NULL(kf->T1);
   
   kf->I = m_get(2, 2);
   ASSERT_NOT_NULL(kf->I);
   m_ident(kf->I);

   /* set initial state: */
   kf->x = v_get(2);
   ASSERT_NOT_NULL(kf->x);
   v_set_val(kf->x, 0, pos);
   v_set_val(kf->x, 1, speed);

   /* no measurement or control yet: */
   kf->z = v_get(2);
   ASSERT_NOT_NULL(kf->z);
   kf->u = v_get(1);
   ASSERT_NOT_NULL(kf->u);

   kf->P = m_get(2, 2);
   ASSERT_NOT_NULL(kf->P);
   m_ident(kf->P);
   
   /* set up noise: */
   kf->Q = m_get(2, 2);
   ASSERT_NOT_NULL(kf->Q);
   sm_mlt(q, kf->I, kf->Q);
   kf->R = m_get(2, 2);
   ASSERT_NOT_NULL(kf->R);
   sm_mlt(r, kf->I, kf->R);
   
   kf->K = m_get(2, 1);
   ASSERT_NOT_NULL(kf->K);
   kf->H = m_get(2, 2);
   ASSERT_NOT_NULL(kf->H);
   m_set_val(kf->H, 0, 0, 1.0);
    
   /* A = | 1.0   dt  |
          | 0.0   1.0 |
      note: dt value is set in kalman_run */
   kf->A = m_get(2, 2);
   ASSERT_NOT_NULL(kf->A);
   m_set_val(kf->A, 0, 0, 1.0);
   m_set_val(kf->A, 1, 0, 0.0);
   m_set_val(kf->A, 1, 1, 1.0);

   /* B = | 0.5 * dt ^ 2 |
          |     dt       |
      dt values are set in kalman_run */
   kf->B = m_get(2, 1);
   ASSERT_NOT_NULL(kf->B);
}


static void kalman_predict(kalman_t *kf, float a)
{
   /* x = A * x + B * u */
   v_set_val(kf->u, 0, a);
   mv_mlt(kf->A, kf->x, kf->t0);
   mv_mlt(kf->B, kf->u, kf->t1);
   v_add(kf->t0, kf->t1, kf->x);

   /* P = A * P * AT + Q */
   m_mlt(kf->A, kf->P, kf->T0);
   mmtr_mlt(kf->T0, kf->A, kf->T1);
   m_add(kf->T1, kf->Q, kf->P);
}



static void kalman_correct(kalman_t *kf, float p)
{
   /* K = P * HT * inv(H * P * HT + R) */
   m_mlt(kf->H, kf->P, kf->T0);
   mmtr_mlt(kf->T0, kf->H, kf->T1);
   m_add(kf->T1, kf->R, kf->T0);
   m_inverse(kf->T0, kf->T1);
   mmtr_mlt(kf->P, kf->H, kf->T0);
   m_mlt(kf->T0, kf->T1, kf->K);

   /* x = x + K * (z - H * x) */
   mv_mlt(kf->H, kf->x, kf->t0);
   v_set_val(kf->z, 0, p);
   v_sub(kf->z, kf->t0, kf->t1);
   mv_mlt(kf->K, kf->t1, kf->t0);
   v_add(kf->x, kf->t0, kf->t1);
   v_copy(kf->t1, kf->x);
   
   /* P = (I - K * H) * P */
   m_mlt(kf->K, kf->H, kf->T0);
   m_sub(kf->I, kf->T0, kf->T1);
   m_mlt(kf->T1, kf->P, kf->T0);
   m_copy(kf->T0, kf->P);
}


/*
 * executes kalman predict and correct step
 */
void kalman_run(kalman_out_t *out, kalman_t *kalman, const kalman_in_t *in)
{
   /* A = | init   dt  |
          | init  init | */
   m_set_val(kalman->A, 0, 1, in->dt);

   /* B = | 0.5 * dt ^ 2 |
          |     dt       | */
   m_set_val(kalman->B, 0, 0, 0.5f * in->dt * in->dt);
   m_set_val(kalman->B, 1, 0, in->dt);

   kalman_predict(kalman, in->acc);
   kalman_correct(kalman, in->pos);
   out->pos = v_entry(kalman->x, 0);
   out->speed = v_entry(kalman->x, 1);
}

