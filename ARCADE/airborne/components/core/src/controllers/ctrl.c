
/*
 * ctrl.c
 *
 * created on: 26.09.2010
 * author: tobi
 */


#include <opcd_params.h>
#include <periodic_thread.h>
#include <threadsafe_types.h>
#include <util.h>
#include <udp4.h>
#include <debug_buffer.h>
#include <debug_protobuf.h>
#include <debug_data.pb-c.h>

#include "navi.h"
#include "ctrl.h"
#include "pid.h"
#include "alt_ctrl.h"
#include "yaw_ctrl.h"
#include "../util/logger/logger.h"
#include "../util/time/ltime.h"
#include "../sensor_actor/interfaces/gps.h"


typedef struct
{
   threadsafe_float_t alt;
   threadsafe_float_t x;
   threadsafe_float_t y;
   
   /* altitude setpoint data: */
   enum 
   {
      ALT_MODE_ULTRA, 
      ALT_MODE_BARO
   } alt_mode;
   
   /* yaw setpoint data: */
   enum 
   {
      YAW_MODE_FIXED, 
      YAW_MODE_POI
   } yaw_mode;
   threadsafe_float_t poi_x;
   threadsafe_float_t poi_y;
}
ctrl_sp_t;


static controller_errors_t errors;

static ctrl_sp_t sp;


static udp_socket_t *udp_socket = NULL;
static debug_buffer_t *debug_buffer = NULL;
static periodic_thread_t debug_thread;

static pid_controller_t pitch_controller;
static pid_controller_t roll_controller;

static threadsafe_float_t angle_p;
static threadsafe_float_t angle_i;
static threadsafe_float_t angle_i_max;
static threadsafe_float_t angle_d;
static threadsafe_float_t pitch_bias;
static threadsafe_float_t roll_bias;
static threadsafe_int_t angle_cal;
static char *debug_host;
static threadsafe_int_t debug_port;



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



static int ctrl_set_yaw_setpoint(float pos, float *speed)
{
   if (speed != NULL)
   {
      int status = yaw_ctrl_set_speed(*speed);
      if (status != 0)
      {
         return status;
      }
   }
   else
   {
      yaw_ctrl_std_speed();   
   }
   return yaw_ctrl_set_pos(pos);
}


/*
 * sets the setpoint for type to val
 * returns 0 on success or EINVAL
 */
int ctrl_set_setpoint(CtrlData *data)
{
   int status = 0;
   switch (data->type)
   {
      case CTRL_TYPE__YAW:
         switch (data->yaw_type)
         {
            case YAW_TYPE__ABSOLUTE:
            {
               status = ctrl_set_yaw_setpoint(data->pos[0], data->speed);
               break;
            }
            case YAW_TYPE__RELATIVE:
            {
               float yaw_sp = yaw_ctrl_get_pos();
               yaw_sp += data->pos[0];
               yaw_sp = normalize_0_2pi(yaw_sp);
               status = ctrl_set_yaw_setpoint(yaw_sp, data->speed);
               break;
            }
            case YAW_TYPE__POI:
               break;
         }
         LOG(LL_DEBUG, "yaw setpoint update: %f", pos[0]);
         break;
 
      case CTRL_TYPE__GPS:
         LOG(LL_DEBUG, "gps (meters) setpoint update: %f, %f", pos[0], pos[1]);
         threadsafe_float_set(&sp.x, pos[0]);
         threadsafe_float_set(&sp.y, pos[1]);
         break;
 
     case CTRL_TYPE__ALT:
         LOG(LL_DEBUG, "ultra setpoint update: %f", pos[0]);
         sp.alt_mode = ALT_MODE_BARO;
         threadsafe_float_set(&sp.alt, pos[0]);
         break;
   }
   return status;
}


/*
 * returns the current error value for type
 */
size_t ctrl_get_err(float *out, CtrlType type)
{
   size_t size;
   switch (type)
   {
      case CTRL_TYPE__ALT:
         out[0] = threadsafe_float_get(&errors.alt_error);
         size = 1;
         break;

      case CTRL_TYPE__GPS:
         out[0] = threadsafe_float_get(&errors.x_error);
         out[1] = threadsafe_float_get(&errors.y_error);
         size = 2;
         break;

      case CTRL_TYPE__YAW:
         out[0] = threadsafe_float_get(&errors.yaw_error);
         size = 1;
         break;
   }

   return size;
}


PERIODIC_THREAD_BEGIN(debug_thread_func)
{
   PERIODIC_THREAD_LOOP_BEGIN
   {
      debug_buffer_lock(debug_buffer);
      binary_data_t bindata = debug_protobuf_pack(debug_buffer);
      debug_buffer_unlock(debug_buffer);
      udp_socket_send(udp_socket, bindata.data, bindata.size);
      free(bindata.data);
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
    * prepare and run controllers:
    */
   float err_x = pos_x - threadsafe_float_get(&sp.x);
   float err_y =  pos_y - threadsafe_float_get(&sp.y);
 
   /*
    * run controllers:
    */
   float yaw_error;
   float yaw_ctrl_val = yaw_ctrl_step(&yaw_error, model_state->yaw.angle,
                                      model_state->yaw.speed, dt);

   float alt_err;
   if (sp.alt_mode == ALT_MODE_ULTRA)
   {
      alt_err = threadsafe_float_get(&sp.alt) - model_state->ultra_z.pos;
   }
   else
   {
      alt_err = threadsafe_float_get(&sp.alt) - model_state->baro_z.pos;
   }
   
   float gas_ctrl_val = alt_ctrl_step(alt_err, model_state->baro_z.speed, dt);

   /* run navigator: */
   navi_input_t navi_input = {threadsafe_float_get(&sp.x),
                              threadsafe_float_get(&sp.y),
                              pos_x, pos_y, speed_x, speed_y,
                              acc_x, acc_y, dt, yaw};
   navi_output_t navi_output;
   navigator_run(&navi_output, &navi_input);

   if (threadsafe_int_get(&angle_cal))
   {
      navi_output.pitch = 0.0;
      navi_output.roll = 0.0;
   }

   /* run pitch/roll angle controllers: */
   float pitch_ctrl_val = pid_control(&pitch_controller,
                             model_state->pitch.angle + symmetric_limit(navi_output.pitch, 0.17)
                             + threadsafe_float_get(&pitch_bias), dt);

   float roll_ctrl_val = -pid_control(&roll_controller,
                             -model_state->roll.angle + symmetric_limit(navi_output.roll, 0.17)
                             + threadsafe_float_get(&roll_bias), dt);

   if (0) //debug_buffer_trylock(debug_buffer) == 0)
   {
      debug_buffer_reset(debug_buffer);
      debug_buffer_add(debug_buffer, KEY_TYPE__PITCH, model_state->pitch.angle);
      debug_buffer_add(debug_buffer, KEY_TYPE__ROLL, model_state->roll.angle);
      debug_buffer_add(debug_buffer, KEY_TYPE__YAW, model_state->yaw.angle);
      debug_buffer_add(debug_buffer, KEY_TYPE__POS_X, model_state->x.pos);
      debug_buffer_add(debug_buffer, KEY_TYPE__POS_Y, model_state->y.pos);
      debug_buffer_add(debug_buffer, KEY_TYPE__POS_Z, model_state->ultra_z.pos);
      debug_buffer_add(debug_buffer, KEY_TYPE__SPEED_X, model_state->x.speed);
      debug_buffer_add(debug_buffer, KEY_TYPE__SPEED_Y, model_state->y.speed);
      debug_buffer_add(debug_buffer, KEY_TYPE__SPEED_Z, model_state->ultra_z.speed);
      debug_buffer_add(debug_buffer, KEY_TYPE__ACC_X, model_state->x.acc);
      debug_buffer_add(debug_buffer, KEY_TYPE__ACC_Y, model_state->y.acc);
      debug_buffer_add(debug_buffer, KEY_TYPE__ACC_Z, model_state->ultra_z.acc);
      debug_buffer_add(debug_buffer, KEY_TYPE__SETPOINT_X, threadsafe_float_get(&sp.x));
      debug_buffer_add(debug_buffer, KEY_TYPE__SETPOINT_Y, threadsafe_float_get(&sp.y));
      debug_buffer_add(debug_buffer, KEY_TYPE__SETPOINT_Z, threadsafe_float_get(&sp.alt));
      debug_buffer_unlock(debug_buffer);
   }

   /* write error state variables: */
   threadsafe_float_set(&errors.x_error, err_x);
   threadsafe_float_set(&errors.y_error, err_y);
   threadsafe_float_set(&errors.alt_error, alt_err);
   threadsafe_float_set(&errors.yaw_error, yaw_error);

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
      data->gas = 1.0; //gas_ctrl_val;
   }
   pthread_mutex_unlock(&override_data.mutex);
}


void ctrl_reset(void)
{
   yaw_ctrl_reset();
   alt_ctrl_reset();
   pid_reset(&pitch_controller);
   pid_reset(&roll_controller);
   navigator_reset(); // TODO: not threadsafe
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
      {"debug_host", &debug_host},
      {"debug_port", &debug_port},
      {"angle.p", &angle_p.value},
      {"angle.i", &angle_i.value},
      {"angle.i_max", &angle_i_max.value},
      {"angle.d", &angle_d.value},
      {"angle.pitch_bias", &pitch_bias},
      {"angle.roll_bias", &roll_bias},
      {"angle.calibrate", &angle_cal},
      OPCD_PARAMS_END
   };
   opcd_params_apply("controllers.", params);

   /* initialize setpoints: */
   threadsafe_float_init(&sp.alt, 1.0f);
   threadsafe_float_init(&sp.x, 0.0f);
   threadsafe_float_init(&sp.y, 0.0f);

   /* initialize errors: */
   threadsafe_float_init(&errors.yaw_error, 0.0f);
   threadsafe_float_init(&errors.alt_error, 0.0f);
   threadsafe_float_init(&errors.x_error, 0.0f);
   threadsafe_float_init(&errors.y_error, 0.0f);

   /* create debug connection: */
   udp_socket = udp_socket_create(debug_host, threadsafe_int_get(&debug_port), 1, 0);
   debug_buffer = debug_buffer_create(128);
   const struct timespec debug_thread_period = {0, 100 * NSEC_PER_MSEC};
   periodic_thread_start(&debug_thread, debug_thread_func, "debug_thread", 0, debug_thread_period, NULL);

   /* initialize controllers and navi: */
   pid_init(&pitch_controller, &angle_p, &angle_i, &angle_d, &angle_i_max);
   pid_init(&roll_controller, &angle_p, &angle_i, &angle_d, &angle_i_max);
   yaw_ctrl_init();
   alt_ctrl_init();
   navigator_init();
}

