

#include <pthread.h>
#include <string.h>


#include <util.h>
#include <serial.h>
#include <simple_thread.h>
#include <opcd_params.h>


#include "rc_dsl_reader.h"
#include "../../../util/logger/logger.h"
#include "../../../filters/median_filter.h"


#define THREAD_NAME       "rc_dsl_reader"
#define THREAD_PRIORITY   0


#define RSSI_MIN RSSI_SCALE(7)


static simple_thread_t thread;
static serialport_t port;
static rc_dsl_t rc_dsl;
static char *dev_path = NULL;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static float channels[RC_DSL_CHANNELS];
static int sig_valid = 0;
static median_filter_t sig_valid_filter;


SIMPLE_THREAD_BEGIN(thread_func)
{
   SIMPLE_THREAD_LOOP_BEGIN
   {
      int b = serial_read_char(&port);
      if (b < 0)
      {
         msleep(1);
      }
      int status = rc_dsl_parse_dsl_data(&rc_dsl, (uint8_t)b);
      if (status < 0)
      {
         LOG(LL_ERROR, "could not parse dsl frame");   
      }
      else if (status == 1)
      {
         pthread_mutex_lock(&mutex);
         sig_valid = rc_dsl.RSSI > RSSI_MIN;
         if (!sig_valid)
         {
            memset(channels, 0, sizeof(channels));
            channels[4] = -1024;
         }
         else
         {
            memcpy(channels, rc_dsl.channels, sizeof(channels));   
         }
         //sig_valid = median_filter_run(&sig_valid_filter, (float)sig_valid) > 0.5 ? 1 : 0;
         pthread_mutex_unlock(&mutex);
      }
   }
   SIMPLE_THREAD_LOOP_END
}
SIMPLE_THREAD_END



int rc_dsl_reader_init(void)
{
   ASSERT_ONCE();
   THROW_BEGIN();
   memset(channels, 0, sizeof(channels));
   opcd_param_t params[] =
   {
      {"serial_port", &dev_path},
      OPCD_PARAMS_END   
   };
   opcd_params_apply("sensors.rc_dsl.", params);
   THROW_ON_ERR(serial_open(&port, dev_path, 38400, 0, 0, 0));
   rc_dsl_init(&rc_dsl);
   median_filter_init(&sig_valid_filter, 100);
   simple_thread_start(&thread, thread_func, THREAD_NAME, THREAD_PRIORITY, NULL);

   THROW_END();
}


int rc_dsl_reader_get(float channels_out[RC_DSL_CHANNELS])
{
   pthread_mutex_lock(&mutex);
   memcpy(channels_out, channels, sizeof(channels));
   int ret = sig_valid ? 0 : -EAGAIN;
   pthread_mutex_unlock(&mutex);
   return ret;
}

