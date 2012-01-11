
/*
 * bmp085.c
 *
 * created on: 11.06.2010
 * author: tobi
 */


#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <sys/time.h>
#include <math.h>

#include "bmp085.h"
#include "util.h"
#include "../lib/i2c/omap_i2c_bus.h"
#include "../../algorithms/sliding_avg.h"
#include "../../util/time/ltime.h"
#include "../../util/threads/simple_thread.h"
#include "../../util/threads/threadsafe_types.h"



#define THREAD_NAME       "bmp085"
#define THREAD_PRIORITY   0

#define HAVE_OMAP_I2C_BUS


static simple_thread_t thread;
static threadsafe_float_t alt;
static sliding_avg_t *avg;

static i2c_dev_t device;
static bmp085_ctx_t context;


SIMPLE_THREAD_BEGIN(thread_func)
{
   bmp085_read_temperature(&device, &context);
   float pressure = bmp085_read_pressure(&device, &context);
   float start_alt = 44330.75 * (1.0 - pow(pressure / 101325.0, 0.19029));

   SIMPLE_THREAD_LOOP_BEGIN
   {
      bmp085_read_temperature(&device, &context);
      pressure = bmp085_read_pressure(&device, &context);
      float _alt = 44330.75 * (1.0 - pow(pressure / 101325.0, 0.19029)) - start_alt;
      _alt = sliding_avg_calc(avg, _alt);
      threadsafe_float_set(&alt, _alt);
      msleep(1);
   }
   SIMPLE_THREAD_LOOP_END
}
SIMPLE_THREAD_END


float bmp085_reader_get_alt(void)
{
   return threadsafe_float_get(&alt);
}


int bmp085_reader_init(void)
{
   ASSERT_ONCE();

   i2c_dev_init(&device, omap_i2c_bus_get(), "bmp085", 0x77);
   int status = bmp085_init(&device, &context);
   if (status != 0)
   {
      return status;   
   }

   avg = sliding_avg_create(20, 0.0f);
   simple_thread_start(&thread, thread_func,
                       THREAD_NAME, THREAD_PRIORITY, NULL);
   return 0;
}


