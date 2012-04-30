
/*
 * ctrl.c
 *
 * created on: 26.09.2010
 * author: tobi
 */


#include <opcd_params.h>
#include <periodic_thread.h>
#include <threadsafe_types.h>
#include <sclhelper.h>
#include <core.pb-c.h>
#include <util.h>

#include "navi.h"
#include "ctrl.h"
#include "pid.h"
#include "alt_ctrl.h"
#include "yaw_ctrl.h"
#include "../util/logger/logger.h"
#include "../util/time/ltime.h"
#include "../sensor_actor/interfaces/gps.h"
#include "../sensor_actor/voltage/voltage_reader.h"


typedef struct
{
   /* relative GPS setpoint: */
   tsfloat_t x;
   tsfloat_t y;
   tsfloat_t z;
   enum 
   {
      ALT_MODE_ULTRA, 
      ALT_MODE_BARO
   }
   alt_mode;
}
ctrl_sp_t;
static ctrl_sp_t sp;


static pthread_mutex_t scl_mutex = PTHREAD_MUTEX_INITIALIZER;
static periodic_thread_t monitor_thread;

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
static void *mon_socket = NULL;
static MonData mon_data = MON_DATA__INIT;
static periodic_thread_t thread;


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



/*
 * sets the setpoint for type to val
 * returns 0 on success or EINVAL
 */
int ctrl_set_data(CtrlParam param, float value)
{
   int status = 0;
   switch (param)
   {
      case CTRL_PARAM__POS_X:
         LOG(LL_DEBUG, "x pos update: %f", value);
         tsfloat_set(&sp.x, value);
         break;

      case CTRL_PARAM__POS_Y:
         LOG(LL_DEBUG, "y pos update: %f", value);
         tsfloat_set(&sp.x, value);
         break;

      case CTRL_PARAM__POS_Z_GROUND:
         LOG(LL_DEBUG, "ground z pos update: %f", value);
         sp.alt_mode = ALT_MODE_ULTRA;
         tsfloat_set(&sp.z, value);
         break;

      case CTRL_PARAM__POS_Z_MSL:
         LOG(LL_DEBUG, "msl z pos update: %f", value);
         sp.alt_mode = ALT_MODE_BARO;
         tsfloat_set(&sp.z, value);
         break;

      case CTRL_PARAM__POS_YAW:
         LOG(LL_DEBUG, "yaw pos update: %f", value);
         status = yaw_ctrl_set_pos(value);
         break;
   }
   return status;
}



PERIODIC_THREAD_BEGIN(mon_emitter)
{
   PERIODIC_THREAD_LOOP_BEGIN
   {
      pthread_mutex_lock(&scl_mutex);
      SCL_PACK_AND_SEND_DYNAMIC(mon_socket, mon_data, mon_data);
      pthread_mutex_unlock(&scl_mutex);
   }
   PERIODIC_THREAD_LOOP_END
}
PERIODIC_THREAD_END



void ctrl_step(mixer_in_t *data, float dt, model_state_t *model_state)
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

   /*
    * run controllers:
    */
   float yaw_err;
   float yaw_ctrl_val = yaw_ctrl_step(&yaw_err, model_state->yaw.angle,
                                      model_state->yaw.speed, dt);

   float z_err;
   if (sp.alt_mode == ALT_MODE_ULTRA)
   {
      z_err = tsfloat_get(&sp.z) - model_state->ultra_z.pos;
   }
   else
   {
      z_err = tsfloat_get(&sp.z) - model_state->baro_z.pos;
   }
   
   float gas_ctrl_val = alt_ctrl_step(z_err, model_state->baro_z.speed, dt);

   navi_input_t navi_input = {tsfloat_get(&sp.x),
                              tsfloat_get(&sp.y),
                              pos_x, pos_y, speed_x, speed_y,
                              acc_x, acc_y, dt, yaw};
   
   navi_output_t navi_output;
   navi_run(&navi_output, &navi_input);
   if (tsint_get(&angle_cal))
   {
      navi_output.pitch = 0.0;
      navi_output.roll = 0.0;
   }

   /* run pitch/roll angle controllers: */
   float _angle_max = tsfloat_get(&angle_max);
   float pitch_ctrl_val = pid_control(&pitch_controller,
                             model_state->pitch.angle + sym_limit(navi_output.pitch, _angle_max)
                             + tsfloat_get(&pitch_bias), dt);

   float roll_ctrl_val = -pid_control(&roll_controller,
                             -model_state->roll.angle + sym_limit(navi_output.roll, _angle_max)
                             + tsfloat_get(&roll_bias), dt);

   if (pthread_mutex_trylock(&scl_mutex) == 0)
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
      
      mon_data.x_err = pos_x - tsfloat_get(&sp.x);
      mon_data.y_err = pos_y - tsfloat_get(&sp.y);
      mon_data.z_err = z_err;
      mon_data.yaw_err = yaw_err;

      mon_data.batt_voltage = voltage_reader_get();
      pthread_mutex_unlock(&scl_mutex);
   }

   /* finally, set actuator outputs: */
   pthread_mutex_lock(&override_data.mutex);
   if (override_data.enabled)
   {
      data->pitch = override_data.pitch;
      data->roll = override_data.roll;
      data->yaw = override_data.yaw;
      data->gas = override_data.gas;
   }
   else
   {
      data->pitch = pitch_ctrl_val;
      data->roll = roll_ctrl_val;
      data->yaw = yaw_ctrl_val;
      data->gas = gas_ctrl_val;
   }
   pthread_mutex_unlock(&override_data.mutex);
}


void ctrl_reset(void)
{
   yaw_ctrl_reset();
   alt_ctrl_reset();
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

   /* initialize setpoints: */
   tsfloat_init(&sp.z, 2.0f);
   tsfloat_init(&sp.x, 0.0f);
   tsfloat_init(&sp.y, 0.0f);

   /* create monitoring connection: */
   const struct timespec period = {0, 100 * NSEC_PER_MSEC};
   periodic_thread_start(&thread, mon_emitter, "mon_thread", 0, period, NULL);

   /* initialize controllers: */
   pid_init(&pitch_controller, &angle_p, &angle_i, &angle_d, &angle_i_max);
   pid_init(&roll_controller, &angle_p, &angle_i, &angle_d, &angle_i_max);
   yaw_ctrl_init();
   alt_ctrl_init();
   navi_init();
}

