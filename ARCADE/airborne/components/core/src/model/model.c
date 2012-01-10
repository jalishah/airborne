
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
#include <kalman.pb-c.h>

#include "model.h"
#include "kalman.h"
#include "body_to_world.h"
#include "../util/logger/logger.h"
#include "../algorithms/sliding_avg.h"
#include "../util/math/lmath.h"


#define THREAD_NAME       "acc_avg_updater"
#define THREAD_PRIORITY   0


/* configuration parameters: */
static threadsafe_float_t process_noise;
static threadsafe_float_t ultra_noise;
static threadsafe_float_t baro_noise;
static threadsafe_float_t gps_noise;
static threadsafe_int_t acc_avg_update_s;
static threadsafe_float_t x_acc_avg_conf;
static threadsafe_float_t y_acc_avg_conf;
static threadsafe_float_t z_acc_avg_conf;

/* kalman filters: */
static kalman_t *ultra_z_kalman;
static kalman_t *baro_z_kalman;
static kalman_t *y_kalman;
static kalman_t *x_kalman;

/* threadsafe state variables: */
static threadsafe_float_t yaw;
static threadsafe_float_t baro_alt;
static threadsafe_float_t ultra_alt;
static threadsafe_float_t x;
static threadsafe_float_t y;

/* averages: */
static sliding_avg_t *y_acc_avg;
static sliding_avg_t *x_acc_avg;
static sliding_avg_t *z_acc_avg;

static body_to_world_t *btw;
static void *socket = NULL;
static simple_thread_t thread;




float model_get_yaw(void)
{
   return threadsafe_float_get(&yaw);
}


float model_get_ultra_alt(void)
{
   return threadsafe_float_get(&ultra_alt);
}


float model_get_baro_alt(void)
{
   return threadsafe_float_get(&baro_alt);
}


float model_get_x(void)
{
   return threadsafe_float_get(&x);
}


float model_get_y(void)
{
   return threadsafe_float_get(&y);
}


void model_step(model_state_t *out, model_input_t *in)
{
   ASSERT_NOT_NULL(out);
   ASSERT_NOT_NULL(in);

   /*
    * transform acceleration vector from body to world coordinate system:
    * the world accelerations are later fused with GPS/Altimeter data
    * for improved system state determination
    */

   euler_angles_t euler_angles =
   {
      in->ahrs_data.pitch,
      in->ahrs_data.roll,
      in->ahrs_data.yaw
   };

   body_vector_t body_vector =
   {
      in->ahrs_data.acc_pitch,
      in->ahrs_data.acc_roll,
      in->ahrs_data.acc_yaw
   };

   world_vector_t world_vector;
   body_to_world_transform(btw, &euler_angles, &body_vector, &world_vector);

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
      in->gps_data.delta_x,
      world_acc_x
   };

   const kalman_in_t y_in =
   {
      in->dt,
      in->gps_data.delta_y,
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

   kalman_run(&y_kalman_out, y_kalman, &y_in);
   kalman_run(&x_kalman_out, x_kalman, &x_in);
   kalman_run(&ultra_z_kalman_out, ultra_z_kalman, &ultra_z_in);
   kalman_run(&baro_z_kalman_out, baro_z_kalman, &baro_z_in);

   /* update model state: */
   out->ultra_z.pos = ultra_z_kalman_out.pos;
   out->ultra_z.speed = ultra_z_kalman_out.speed;
   out->ultra_z.acc = world_acc_z;
   
   out->baro_z.pos = baro_z_kalman_out.pos;
   out->baro_z.speed = baro_z_kalman_out.speed;
   out->baro_z.acc = world_acc_z;
   
   out->yaw.angle = normalize_euler_0_2pi(in->ahrs_data.yaw);
   out->yaw.speed = in->ahrs_data.yaw_rate;

   out->pitch.angle = in->ahrs_data.pitch;
   out->pitch.speed = in->ahrs_data.pitch_rate;

   out->roll.angle = in->ahrs_data.roll;
   out->roll.speed = in->ahrs_data.roll_rate;

   out->x.pos = x_kalman_out.pos;
   out->x.speed = x_kalman_out.speed;
   out->x.acc = world_acc_x;

   out->y.pos = y_kalman_out.pos;
   out->y.speed = y_kalman_out.speed;
   out->y.acc = world_acc_y;

   KalmanData kalman_data = KALMAN_DATA__INIT;
   kalman_data.lon = out->x.pos; //in->gps_data.start_x;
   kalman_data.lat = out->y.pos; //in->gps_data.start_y;
   EVERY_N_TIMES(50,
                 unsigned int data_len = (unsigned int)kalman_data__get_packed_size(&kalman_data);
                 void *buffer = malloc(data_len);
                 kalman_data__pack(&kalman_data, buffer);
                 scl_send_dynamic(socket, buffer, data_len, ZMQ_NOBLOCK);
                );
  
   /* set accessable state variables: */
   threadsafe_float_set(&yaw, out->yaw.angle);
   threadsafe_float_set(&x, out->x.pos);
   threadsafe_float_set(&y, out->y.pos);
   threadsafe_float_set(&baro_alt, out->baro_z.pos);
   threadsafe_float_set(&ultra_alt, out->ultra_z.pos);

}


SIMPLE_THREAD_BEGIN(thread_func)
{
   SIMPLE_THREAD_LOOP_BEGIN
   {
      sleep(1); //sleep(threadsafe_int_get(&acc_avg_update_s));
      opcd_float_param_set("core.model.x_acc_avg", sliding_avg_get(x_acc_avg));
      opcd_float_param_set("core.model.y_acc_avg", sliding_avg_get(y_acc_avg));
      opcd_float_param_set("core.model.z_acc_avg", sliding_avg_get(z_acc_avg));
   }
   SIMPLE_THREAD_LOOP_END
}
SIMPLE_THREAD_END


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
       threadsafe_float_get(&process_noise),
       threadsafe_float_get(&ultra_noise),
       threadsafe_float_get(&baro_noise),
       threadsafe_float_get(&gps_noise));


   socket = scl_get_socket("kalman");
   
   /* set-up kalman filters: */
   kalman_out_t kalman_state = {0.0, 0.0};
   
   kalman_config_t alt_kalman_config = 
   {
      threadsafe_float_get(&process_noise),
      threadsafe_float_get(&ultra_noise)
   };
   
   kalman_config_t baro_kalman_config = 
   {
      threadsafe_float_get(&process_noise), 
      threadsafe_float_get(&baro_noise)
   };
   
   kalman_config_t lateral_kalman_config = 
   {
      threadsafe_float_get(&process_noise),
      threadsafe_float_get(&gps_noise)
   };

   /* create kalman filters: */
   y_kalman = kalman_create(&lateral_kalman_config, &kalman_state);
   x_kalman = kalman_create(&lateral_kalman_config, &kalman_state);
   ultra_z_kalman = kalman_create(&alt_kalman_config, &kalman_state);
   baro_z_kalman = kalman_create(&baro_kalman_config, &kalman_state);
   
   /* initialize accessable state variables: */
   threadsafe_float_init(&yaw, 0.0);
   threadsafe_float_init(&baro_alt, 0.0);
   threadsafe_float_init(&ultra_alt, 0.0);
   threadsafe_float_init(&x, 0.0);
   threadsafe_float_init(&y, 0.0);

   /* set-up body to world coordinate transformation: */
   btw = body_to_world_create();

   /* intitialize averages: */
   const int ACC_AVG_SIZE = 10000;
   x_acc_avg = sliding_avg_create(ACC_AVG_SIZE, threadsafe_float_get(&x_acc_avg_conf));
   y_acc_avg = sliding_avg_create(ACC_AVG_SIZE, threadsafe_float_get(&y_acc_avg_conf));
   z_acc_avg = sliding_avg_create(ACC_AVG_SIZE, threadsafe_float_get(&z_acc_avg_conf));
   
   /* start acc avg updater: */
   simple_thread_start(&thread, thread_func, THREAD_NAME, THREAD_PRIORITY, NULL);
}

