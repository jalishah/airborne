

#include <pthread.h>
#include <string.h>
#include <serial.h>
#include <simple_thread.h>
#include <opcd_params.h>


#include "rc_dsl_driver.h"
#include "../../libs/rc_dsl/rc_dsl.h"


#define THREAD_NAME       "rc_dsl"
#define THREAD_PRIORITY   0


static simple_thread_t thread;


static serialport_t port;
static rc_dsl_t *rc_dsl = NULL;
static char *dev_path = NULL;
static rc_data_t data;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


SIMPLE_THREAD_BEGIN(thread_func)
{
   SIMPLE_THREAD_LOOP_BEGIN
   {
      uint8_t b = serial_read_char(&port);
      if (rc_dsl_parse_dsl_data(rc_dsl, b) == 1)
      {
         pthread_mutex_lock(&mutex);
         // TODO
         pthread_mutex_unlock(&mutex);
      }
   }
   SIMPLE_THREAD_LOOP_END
}
SIMPLE_THREAD_END


int rc_dsl_driver_init(void)
{
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



void rc_dsl_driver_read(rc_data_t *data_out)
{
   pthread_mutex_lock(&mutex);
   *data_out = data;
   pthread_mutex_unlock(&mutex);
}

