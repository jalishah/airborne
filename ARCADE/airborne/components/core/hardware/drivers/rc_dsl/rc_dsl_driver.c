

#include <pthread.h>
#include <string.h>


#include <stdbool.h>
#include <util.h>
#include <serial.h>
#include <simple_thread.h>
#include <opcd_params.h>


#include "rc_dsl_driver.h"
#include "../../libs/rc_dsl/rc_dsl.h"
#include "../../../util/logger/logger.h"


#define THREAD_NAME       "rc_dsl"
#define THREAD_PRIORITY   0

#define CENTER_SCALE(x) (((float)x) / 2000.0f)
#define ABS_SCALE(x) (((float)x) / 4000.0f + 0.5f)
#define RSSI_SCALE(x) (((float)x) / 255.0f)
#define RSSI_MIN RSSI_SCALE(7)


static simple_thread_t thread;
static serialport_t port;
static rc_dsl_t *rc_dsl = NULL;
static char *dev_path = NULL;
static rc_data_t data;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static float pitch_raw = 0.0;
static float roll_raw = 0.0;
static float yaw_raw = 0.0;
static float pitch_bias = 0.0;
static float roll_bias = 0.0;
static float yaw_bias = 0.0;
static bool sig_valid = false;


SIMPLE_THREAD_BEGIN(thread_func)
{
   SIMPLE_THREAD_LOOP_BEGIN
   {
      int b = serial_read_char(&port);
      if (b < 0)
      {
         msleep(10);
      }
      int status = rc_dsl_parse_dsl_data(rc_dsl, (uint8_t)b);
      if (status < 0)
      {
         LOG(LL_ERROR, "could not parse dsl frame");   
      }
      else if (status == 1)
      {
         pthread_mutex_lock(&mutex);
         data.rssi = RSSI_SCALE(rc_dsl_get_rssi(rc_dsl));
         if (data.rssi > RSSI_MIN)
         {
            sig_valid = true;
            pitch_raw = CENTER_SCALE(rc_dsl_get_channel(rc_dsl, 0));
            roll_raw = CENTER_SCALE(rc_dsl_get_channel(rc_dsl, 1));
            yaw_raw = CENTER_SCALE(rc_dsl_get_channel(rc_dsl, 3));
            data.pitch = pitch_raw - pitch_bias;
            data.roll = roll_raw - roll_bias;
            data.yaw = yaw_raw - yaw_bias;
            data.gas = ABS_SCALE(rc_dsl_get_channel(rc_dsl, 2));
            data.extra[0] = ABS_SCALE(rc_dsl_get_channel(rc_dsl, 4));
         }
         else
         {
            sig_valid = false;
         }
         pthread_mutex_unlock(&mutex);
      }
   }
   SIMPLE_THREAD_LOOP_END
}
SIMPLE_THREAD_END



int rc_dsl_driver_init(void)
{
   ASSERT_ONCE();
   int status = 0;
   memset(&data, 0, sizeof(rc_data_t));
   opcd_param_t params[] =
   {
      {"serial_port", &dev_path},
      OPCD_PARAMS_END   
   };
   opcd_params_apply("sensors.rc_dsl.", params);
   if ((status = serial_open(&port, dev_path, 38400, 0, 0, 0)) != 0)
   {
      goto out;
   }
   rc_dsl = rc_dsl_create();
   simple_thread_start(&thread, thread_func, THREAD_NAME, THREAD_PRIORITY, NULL);

out:
   return status;
}


int rc_dsl_driver_calibrate(void)
{
   int status = 0;
   float _pitch_bias;
   float _roll_bias;
   float _yaw_bias;
   int max_count = 100;
   int valid_count = 0;

   /* collect raw samples:  */
   FOR_N(i, max_count)
   {
      pthread_mutex_lock(&mutex);
      if (sig_valid)
      {
         valid_count++;
         _pitch_bias += pitch_raw;
         _roll_bias += roll_raw;
         _yaw_bias += yaw_raw;
      }
      pthread_mutex_unlock(&mutex);
      msleep(10);
   }

   /* if we have enough valid samples, set bias values: */
   if ((float)valid_count > 0.8f * (float)max_count)
   {
      pthread_mutex_lock(&mutex);
      pitch_bias = _pitch_bias / max_count;
      roll_bias = _roll_bias / max_count;
      yaw_bias = _yaw_bias / max_count;
      pthread_mutex_unlock(&mutex);
   }
   else
   {
      status = -1;   
   }
   return status;
}


void rc_dsl_driver_read(rc_data_t *data_out)
{
   pthread_mutex_lock(&mutex);
   *data_out = data;
   pthread_mutex_unlock(&mutex);
}

