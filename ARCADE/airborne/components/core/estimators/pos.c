
/*
   Kalman Filter based Position/Speed Estimate
   
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
#include <simple_thread.h>
#include <opcd_params.h>
#include <threadsafe_types.h>
#include <sclhelper.h>

#include "pos.h"
#include "../geometry/orientation.h"
#include "../filters/sliding_avg.h"
#include "../util/logger/logger.h"


/* configuration parameters: */
static tsfloat_t process_noise;
static tsfloat_t ultra_noise;
static tsfloat_t baro_noise;
static tsfloat_t gps_noise;
static tsint_t acc_avg_update_s;
static tsfloat_t acc_avg_start[3];


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


/* static function prototypes: */
static void kalman_init(kalman_t *kf, float q, float r, float pos, float speed);
static void kalman_run(kalman_t *kf, float *est_pos, float *est_speed, float pos, float acc, float dt);


/* kalman filters: */
static kalman_t ultra_z_kalman;
static kalman_t baro_z_kalman;
static kalman_t y_kalman;
static kalman_t x_kalman;

/* averages: */
static sliding_avg_t acc_avgs[3];


void pos_init(void)
{
   ASSERT_ONCE();

   /* read configuration and initialize scl gates: */
   opcd_param_t params[] =
   {
      {"process_noise", &process_noise},
      {"ultra_noise", &ultra_noise},
      {"baro_noise", &baro_noise},
      {"gps_noise", &gps_noise},
      {"acc_avg_update_s", &acc_avg_update_s},
      {"x_acc_avg", &acc_avg_start[0]},
      {"y_acc_avg", &acc_avg_start[1]},
      {"z_acc_avg", &acc_avg_start[2]},
      OPCD_PARAMS_END
   };
   opcd_params_apply("kalman_pos.", params);
   LOG(LL_DEBUG, "process noise: %f, ultra noise: %f, baro noise: %f, gps noise: %f\n",
       tsfloat_get(&process_noise),
       tsfloat_get(&ultra_noise),
       tsfloat_get(&baro_noise),
       tsfloat_get(&gps_noise));

   /* set-up kalman filters: */
   kalman_init(&x_kalman, tsfloat_get(&process_noise), tsfloat_get(&gps_noise), 0, 0);
   kalman_init(&y_kalman, tsfloat_get(&process_noise), tsfloat_get(&gps_noise), 0, 0);
   kalman_init(&ultra_z_kalman, tsfloat_get(&process_noise), tsfloat_get(&ultra_noise), 0, 0);
   kalman_init(&baro_z_kalman, tsfloat_get(&process_noise), tsfloat_get(&baro_noise), 0, 0);
   
   /* intitialize averages: */
   const int ACC_AVG_SIZE = 10000;
   FOR_N(i, 3)
   {
      sliding_avg_init(&acc_avgs[i], ACC_AVG_SIZE, tsfloat_get(&acc_avg_start[i]));
   }
}


void pos_update(pos_t *out, pos_in_t *in)
{
   ASSERT_NOT_NULL(out);
   ASSERT_NOT_NULL(in);

   /* center global ACC readings by sliding average: */
   vec3_t world_acc;
   FOR_N(i, 3)
   {
      float avg = sliding_avg_calc(&acc_avgs[i], in->acc.vec[i]);
      world_acc.vec[i] = in->acc.vec[i] - avg;
   }

   /* run kalman filters: */
   kalman_run(&x_kalman,       &out->x.pos,       &out->x.speed,       in->dx,      world_acc.x, in->dt);
   kalman_run(&y_kalman,       &out->y.pos,       &out->y.speed,       in->dy,      world_acc.y, in->dt);
   kalman_run(&ultra_z_kalman, &out->ultra_z.pos, &out->ultra_z.speed, in->ultra_z, world_acc.z, in->dt);
   kalman_run(&baro_z_kalman,  &out->baro_z.pos,  &out->baro_z.speed,  in->baro_z,  world_acc.z, in->dt);
}


static void kalman_init(kalman_t *kf, float q, float r, float pos, float speed)
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
   v_set_val(kf->z, 1, 0.0f);
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
static void kalman_run(kalman_t *kf, float *est_pos, float *est_speed, float pos, float acc, float dt)
{
   /* A = | init   dt  |
          | init  init | */
   m_set_val(kf->A, 0, 1, dt);

   /* B = | 0.5 * dt ^ 2 |
          |     dt       | */
   m_set_val(kf->B, 0, 0, 0.5f * dt * dt);
   m_set_val(kf->B, 1, 0, dt);

   kalman_predict(kf, acc);
   kalman_correct(kf, pos);
   *est_pos = v_entry(kf->x, 0);
   *est_speed = v_entry(kf->x, 1);
}

