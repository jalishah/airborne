

#include <pthread.h>
#include <string.h>


#include <stdbool.h>
#include <util.h>
#include <serial.h>
#include <simple_thread.h>
#include <opcd_params.h>


#include "rc_dsl_reader.h"
#include "../../../util/logger/logger.h"


#define THREAD_NAME       "rc_dsl_reader"
#define THREAD_PRIORITY   0


#define RSSI_MIN RSSI_SCALE(7)


static simple_thread_t thread;
static serialport_t port;
static rc_dsl_t rc_dsl;
static char *dev_path = NULL;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static float channels[RC_DSL_CHANNELS];
static int sig_valid = false;


SIMPLE_THREAD_BEGIN(thread_func)
{
   SIMPLE_THREAD_LOOP_BEGIN
   {
      int b = serial_read_char(&port);
      if (b < 0)
      {
         msleep(10);
      }
      int status = rc_dsl_parse_dsl_data(&rc_dsl, (uint8_t)b);
      if (status < 0)
      {
         LOG(LL_ERROR, "could not parse dsl frame");   
      }
      else if (status == 1)
      {
         pthread_mutex_lock(&mutex);
         if (rc_dsl.RSSI > RSSI_MIN)
         {
            sig_valid = 1;
            memcpy(channels, rc_dsl.channels, sizeof(channels));
         }
         else
         {
            sig_valid = 0;
         }
         pthread_mutex_unlock(&mutex);
      }
   }
   SIMPLE_THREAD_LOOP_END
}
SIMPLE_THREAD_END



int rc_dsl_reader_init(void)
{
   ASSERT_ONCE();
   memset(channels, 0, sizeof(channels));
   opcd_param_t params[] =
   {
      {"serial_port", &dev_path},
      OPCD_PARAMS_END   
   };
   opcd_params_apply("sensors.rc_dsl.", params);
   int ret;
   if ((ret = serial_open(&port, dev_path, 38400, 0, 0, 0)) != 0)
   {
      LOG(LL_ERROR, "could not open dsl serial port");
      goto out;
   }
   rc_dsl_init(&rc_dsl);
   simple_thread_start(&thread, thread_func, THREAD_NAME, THREAD_PRIORITY, NULL);

out:
   return ret;
}


int rc_dsl_reader_get(float channels_out[RC_DSL_CHANNELS])
{
   pthread_mutex_lock(&mutex);
   memcpy(channels_out, channels, sizeof(channels));
   int ret = sig_valid ? 0 : -EAGAIN;
   pthread_mutex_unlock(&mutex);
   return ret;
}

