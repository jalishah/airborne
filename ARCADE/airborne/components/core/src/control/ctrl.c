
/*
 * File: ctrl.c
 * Type: set of PID controllers
 * Purpose: runs multiple controllers (x,y,z,yaw) and executes attitude control
 * Design Pattern: Singleton using ctrl_init
 *
 * Responsibilities:
 *   - self-configuration through OPCD
 *   - sub-controller management
 *   - controller state management
 *
 * Author: Tobias Simon, Ilmenau University of Technology
 */


#include <stdint.h>
#include <opcd_params.h>
#include <periodic_thread.h>
#include <threadsafe_types.h>
#include <sclhelper.h>
#include <core.pb-c.h>
#include <util.h>

#include "navi.h"
#include "ctrl.h"
#include "pid.h"
#include "z_ctrl.h"
#include "yaw_ctrl.h"
#include "../util/logger/logger.h"
#include "../util/time/ltime.h"
//#include "../sensor_actor/interfaces/gps.h"
//#include "../sensor_actor/voltage/voltage_reader.h"


static pid_controller_t pitch_controller;
static pid_controller_t roll_controller;

static tsfloat_t angle_p;
static tsfloat_t angle_i;
static tsfloat_t angle_i_max;
static tsfloat_t angle_d;
static tsfloat_t pitch_bias;
static tsfloat_t roll_bias;
static tsint_t angle_cal;
static tsfloat_t angle_max;


/* mon data emitter thread: */

static pthread_mutex_t mon_data_mutex = PTHREAD_MUTEX_INITIALIZER;
static void *mon_socket = NULL;
static MonData mon_data = MON_DATA__INIT;
static periodic_thread_t emitter_thread;


PERIODIC_THREAD_BEGIN(mon_emitter)
{
   PERIODIC_THREAD_LOOP_BEGIN
   {
      pthread_mutex_lock(&mon_data_mutex);
      SCL_PACK_AND_SEND_DYNAMIC(mon_socket, mon_data, mon_data);
      pthread_mutex_unlock(&mon_data_mutex);
   }
   PERIODIC_THREAD_LOOP_END
}
PERIODIC_THREAD_END



static struct
{
   float pitch;
   float roll;
   float yaw;
   float gas;
   int enabled;
   pthread_mutex_t mutex;
} 
override_data = {0.0f, 0.0f, 0.0f, 0.0f, 0, PTHREAD_MUTEX_INITIALIZER};



void ctrl_step(ctrl_out_t *out, float dt, model_state_t *model_state)
{
   /*
    * just some shortcut definitions:
    */
   float pos_x = model_state->x.pos;
   float pos_y = model_state->y.pos;
   float speed_x = model_state->x.speed;
   float speed_y = model_state->y.speed;
   float acc_x = model_state->x.acc;
   float acc_y = model_state->y.acc;
   float yaw = model_state->yaw.angle;

   /* run yaw controller: */
   float yaw_err;
   float yaw_ctrl_val = yaw_ctrl_step
   (
      &yaw_err, model_state->yaw.angle,
      model_state->yaw.speed, dt
   );

   /* run z controller: */
   float z_err;
   float gas_ctrl_val = z_ctrl_step
   (
      &z_err, model_state->ultra_z.pos,
      model_state->baro_z.pos,
      model_state->baro_z.speed, dt
   );

   /* run navi controller: */
   navi_output_t navi_output;
   navi_input_t navi_input = 
   {
      pos_x, pos_y, speed_x, speed_y,
      acc_x, acc_y, dt, yaw
   };
   navi_run(&navi_output, &navi_input);
   
   
   /* run pitch / roll angle controllers: */
   if (tsint_get(&angle_cal))
   {
      navi_output.pitch = 0.0;
      navi_output.roll = 0.0;
   }
   float _angle_max = tsfloat_get(&angle_max);
   float pitch_ctrl_val = pid_control
   (
      &pitch_controller,
      model_state->pitch.angle + sym_limit(navi_output.pitch, _angle_max)
      + tsfloat_get(&pitch_bias), dt
   );
   float roll_ctrl_val = -pid_control
   (
      &roll_controller,
      -model_state->roll.angle + sym_limit(navi_output.roll, _angle_max)
      + tsfloat_get(&roll_bias), dt
   );

   /* set monitoring data: */
   if (pthread_mutex_trylock(&mon_data_mutex) == 0)
   {
      mon_data.pitch = model_state->pitch.angle;
      mon_data.roll = model_state->roll.angle;
      mon_data.yaw = model_state->yaw.angle;
      mon_data.pitch_speed = model_state->pitch.speed;
      mon_data.roll_speed = model_state->roll.speed;
      mon_data.yaw_speed = model_state->yaw.speed;
      mon_data.x = model_state->x.pos;
      mon_data.y = model_state->y.pos;
      mon_data.z = model_state->baro_z.pos;
      mon_data.z_ground = model_state->ultra_z.pos;
      mon_data.x_speed = model_state->x.speed;
      mon_data.y_speed = model_state->y.speed;
      mon_data.z_speed = model_state->baro_z.speed;
      mon_data.x_acc = model_state->x.acc;
      mon_data.y_acc = model_state->y.acc;
      mon_data.z_acc = model_state->baro_z.acc;
      mon_data.x_err = pos_x - navi_get_dest_x();
      mon_data.y_err = pos_y - navi_get_dest_y();
      mon_data.z_err = z_err;
      mon_data.yaw_err = yaw_err;
      mon_data.dt = dt;
      pthread_mutex_unlock(&mon_data_mutex);
   }

   /* finally, set actuator outputs: */
   pthread_mutex_lock(&override_data.mutex);
   if (override_data.enabled)
   {
      out->pitch = override_data.pitch;
      out->roll = override_data.roll;
      out->yaw = override_data.yaw;
      out->gas = override_data.gas;
   }
   else
   {
      out->pitch = pitch_ctrl_val;
      out->roll = roll_ctrl_val;
      out->yaw = yaw_ctrl_val;
      out->gas = gas_ctrl_val;
   }
   pthread_mutex_unlock(&override_data.mutex);
}


void ctrl_reset(void)
{
   yaw_ctrl_reset();
   z_ctrl_reset();
   pid_reset(&pitch_controller);
   pid_reset(&roll_controller);
   navi_reset(); // TODO: not threadsafe
}


void ctrl_override(float pitch, float roll, float yaw, float gas)
{
   pthread_mutex_lock(&override_data.mutex);
   override_data.pitch = pitch;
   override_data.roll = roll;
   override_data.yaw = yaw;
   override_data.gas = gas;
   override_data.enabled = 1;
   pthread_mutex_unlock(&override_data.mutex);
}


void ctrl_stop_override(void)
{
   pthread_mutex_lock(&override_data.mutex);
   override_data.enabled = 0;
   pthread_mutex_unlock(&override_data.mutex);
}


void ctrl_init(void)
{
   ASSERT_ONCE();

   /* load parameters: */
   opcd_param_t params[] =
   {
      {"p", &angle_p.value},
      {"i", &angle_i.value},
      {"i_max", &angle_i_max.value},
      {"d", &angle_d.value},
      {"pitch_bias", &pitch_bias},
      {"roll_bias", &roll_bias},
      {"calibrate", &angle_cal},
      {"angle_max", &angle_max},
      OPCD_PARAMS_END
   };
   opcd_params_apply("controllers.attitude.", params);
   
   mon_socket = scl_get_socket("mon");
   ASSERT_NOT_NULL(mon_socket);
   int64_t hwm = 1;
   zmq_setsockopt(mon_socket, ZMQ_HWM, &hwm, sizeof(hwm));

   /* create monitoring connection: */
   const struct timespec period = {0, 100 * NSEC_PER_MSEC};
   periodic_thread_start(&emitter_thread, mon_emitter, "mon_thread", 0, period, NULL);

   /* initialize controllers: */
   pid_init(&pitch_controller, &angle_p, &angle_i, &angle_d, &angle_i_max);
   pid_init(&roll_controller, &angle_p, &angle_i, &angle_d, &angle_i_max);
   yaw_ctrl_init();
   z_ctrl_init();
   navi_init();
}

