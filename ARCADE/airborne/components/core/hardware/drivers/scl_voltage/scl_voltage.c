

#include <unistd.h>

#include <util.h>
#include <simple_thread.h>
#include <power.pb-c.h>
#include <sclhelper.h>

#include "scl_voltage.h"
#include "../../../filters/lowhi.h"
#include "../../../util/logger/logger.h"


static simple_thread_t thread;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static void *socket;
static float voltage = 16.0;


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
   filt2nd_t filter;
   filter_lp_init(&filter, 0.1f, 0.95f, 1.0f, 1.0f);
   filter.z2[0] =       filter.b * voltage - filter.a2 * voltage;
   filter.z1[0] = 2.0 * filter.b * voltage - filter.a1 * voltage + filter.z2[0];

   /* battery reading loop: */
   SIMPLE_THREAD_LOOP_BEGIN
   {
      float voltage_raw = scl_read_voltage();
      if (voltage_raw < 17.0 && voltage_raw > 10.0)
      {
         pthread_mutex_lock(&mutex);
         filter_lp_run(&filter, &voltage_raw, &voltage);
         pthread_mutex_unlock(&mutex);
      }
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
   pthread_mutex_unlock(&mutex);
}

