

#include <unistd.h>

#include <util.h>
#include <simple_thread.h>
#include <power.pb-c.h>
#include <sclhelper.h>

#include "scl_voltage.h"
#include "../../../filters/filter.h"
#include "../../../util/logger/logger.h"


static simple_thread_t thread;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static void *socket;
static float voltage = 16.0;
static int status = -EAGAIN;


static float scl_read_voltage(void)
{
   float voltage;
   PowerState *state;
   SCL_RECV_AND_UNPACK_DYNAMIC(state, socket, power_state);
   if (state)
   {
      voltage = state->voltage;
      SCL_FREE(power_state, state);
   }
   else
   {
      sleep(1);
      LOG(LL_ERROR, "could not read voltage");
      voltage = 0.0;
   }
   return voltage;
}


SIMPLE_THREAD_BEGIN(thread_func)
{
   /* initial measurement: */
   voltage = scl_read_voltage();
   
   /* filter set-up: */
   Filter1 filter;
   filter1_lp_init(&filter, 0.1f, 1.0f, 1);
   filter.z[0] = voltage;

   /* battery reading loop: */
   SIMPLE_THREAD_LOOP_BEGIN
   {
      float voltage_raw = scl_read_voltage();
      pthread_mutex_lock(&mutex);
      if (voltage_raw < 17.0 && voltage_raw > 10.0)
      {
         filter1_run(&filter, &voltage_raw, &voltage);
         status = 0;
      }
      else
      {
         status = -EIO;   
      }
      pthread_mutex_unlock(&mutex);
   }
   SIMPLE_THREAD_LOOP_END
}
SIMPLE_THREAD_END


int scl_voltage_init(void)
{
   socket = scl_get_socket("power");
   if (socket == NULL)
   {
      return -1;
   }
   simple_thread_start(&thread, thread_func, "voltage_reader", 0, NULL);
   return 0;
}


int scl_voltage_read(float *voltage_out)
{
   pthread_mutex_lock(&mutex);
   *voltage_out = voltage;
   int _status = status;
   pthread_mutex_unlock(&mutex);
   return _status;
}

