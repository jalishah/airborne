
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
#include "../geometry/body_to_world.h"
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
static tsfloat_t x_acc_avg_conf;
static tsfloat_t y_acc_avg_conf;
static tsfloat_t z_acc_avg_conf;

/* kalman filters: */
static kalman_t ultra_z_kalman;
static kalman_t baro_z_kalman;
static kalman_t y_kalman;
static kalman_t x_kalman;

/* averages: */
static sliding_avg_t *y_acc_avg;
static sliding_avg_t *x_acc_avg;
static sliding_avg_t *z_acc_avg;

static body_to_world_t *btw;


void model_step(model_state_t *out, model_input_t *in)
{
   ASSERT_NOT_NULL(out);
   ASSERT_NOT_NULL(in);

   world_vector_t world_vector;

   world_vector.x_dir = in->acc.x;
   world_vector.y_dir = in->acc.y;
   world_vector.z_dir = in->acc.z;

   sliding_avg_calc(x_acc_avg, world_vector.x_dir);
   sliding_avg_calc(y_acc_avg, world_vector.y_dir);
   sliding_avg_calc(z_acc_avg, world_vector.z_dir);

   float world_acc_x = world_vector.x_dir - sliding_avg_get(x_acc_avg);
   float world_acc_y = world_vector.y_dir - sliding_avg_get(y_acc_avg);
   float world_acc_z = world_vector.z_dir - sliding_avg_get(z_acc_avg);

   /* set-up kalman filter inputs: */
   const kalman_in_t x_in =
   {
      in->dt,
      in->dx,
      world_acc_x
   };

   const kalman_in_t y_in =
   {
      in->dt,
      in->dy,
      world_acc_y
   };

   const kalman_in_t ultra_z_in =
   {
      in->dt,
      in->ultra_z,
      -world_acc_z
   };

   const kalman_in_t baro_z_in =
   {
      in->dt,
      in->baro_z,
      -world_acc_z
   };

   /* run kalman filters: */
   kalman_out_t y_kalman_out;
   kalman_out_t x_kalman_out;
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
      {"x_acc_avg", &x_acc_avg_conf},
      {"y_acc_avg", &y_acc_avg_conf},
      {"z_acc_avg", &z_acc_avg_conf},
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
   
   /* set-up body to world coordinate transformation: */
   btw = body_to_world_create();

   /* intitialize averages: */
   const int ACC_AVG_SIZE = 10000;
   x_acc_avg = sliding_avg_create(ACC_AVG_SIZE, tsfloat_get(&x_acc_avg_conf));
   y_acc_avg = sliding_avg_create(ACC_AVG_SIZE, tsfloat_get(&y_acc_avg_conf));
   z_acc_avg = sliding_avg_create(ACC_AVG_SIZE, tsfloat_get(&z_acc_avg_conf));
}

