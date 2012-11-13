
/*
 * model.c
 *
 *  Created on: 17.06.2010
 *      Author: tobi
 *
 * purpose:
 *
 * - reads sensor data and performs sensor fusion
 * - provides estimated system states for controllers
 *
 */


#include <unistd.h>

#include <util.h>
#include <simple_thread.h>
#include <opcd_params.h>
#include <threadsafe_types.h>
#include <sclhelper.h>

#include "model.h"
#include "../geometry/orientation.h"
#include "../filters/kalman.h"
#include "../filters/sliding_avg.h"
#include "../util/logger/logger.h"


/* configuration parameters: */
static tsfloat_t process_noise;
static tsfloat_t ultra_noise;
static tsfloat_t baro_noise;
static tsfloat_t gps_noise;
static tsint_t acc_avg_update_s;
static tsfloat_t acc_avg_start[3];

/* kalman filters: */
static kalman_t ultra_z_kalman;
static kalman_t baro_z_kalman;
static kalman_t y_kalman;
static kalman_t x_kalman;

/* averages: */
static sliding_avg_t acc_avgs[3];



void model_step(model_state_t *out, model_input_t *in)
{
   ASSERT_NOT_NULL(out);
   ASSERT_NOT_NULL(in);

   vec3_t world_acc;
   FOR_N(i, 3)
   {
      float avg = sliding_avg_calc(&acc_avgs[i], in->acc.vec[i]);
      world_acc.vec[i] = in->acc.vec[i] - avg;
   }

   /* set-up kalman filter inputs: */
   const kalman_in_t x_in =
   {
      in->dt,
      in->dx,
      world_acc.x
   };

   const kalman_in_t y_in =
   {
      in->dt,
      in->dy,
      world_acc.y
   };

   const kalman_in_t ultra_z_in =
   {
      in->dt,
      in->ultra_z,
      -world_acc.z
   };

   const kalman_in_t baro_z_in =
   {
      in->dt,
      in->baro_z,
      -world_acc.z
   };

   /* run kalman filters: */
   kalman_out_t x_kalman_out;
   kalman_out_t y_kalman_out;
   kalman_out_t ultra_z_kalman_out;
   kalman_out_t baro_z_kalman_out;

   kalman_run(&y_kalman_out, &y_kalman, &y_in);
   kalman_run(&x_kalman_out, &x_kalman, &x_in);
   kalman_run(&ultra_z_kalman_out, &ultra_z_kalman, &ultra_z_in);
   kalman_run(&baro_z_kalman_out, &baro_z_kalman, &baro_z_in);

   /* update model state: */
   out->ultra_z.pos = ultra_z_kalman_out.pos;
   out->ultra_z.speed = ultra_z_kalman_out.speed;
   
   out->baro_z.pos = baro_z_kalman_out.pos;
   out->baro_z.speed = baro_z_kalman_out.speed;

   out->x.pos = x_kalman_out.pos;
   out->x.speed = x_kalman_out.speed;

   out->y.pos = y_kalman_out.pos;
   out->y.speed = y_kalman_out.speed;
}


void model_init(void)
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
   opcd_params_apply("model.", params);
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

