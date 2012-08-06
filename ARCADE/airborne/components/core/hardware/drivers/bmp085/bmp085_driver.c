
/*
 * bmp085_driver.c
 *
 * created: 05.08.2012
 * author: Tobias Simon, Ilmenau University of Technology
 */



#include <time.h>
#include <stdio.h>
#include <sys/time.h>
#include <math.h>

#include <simple_thread.h>
#include <threadsafe_types.h>
#include <util.h>

#include "bmp085_driver.h"
#include "../../libs/bmp085/bmp085.h"
#include "../../../filters/sliding_avg.h"
#include "../../../util/time/ltime.h"


#define THREAD_NAME       "bmp085"
#define THREAD_PRIORITY   0


static simple_thread_t thread;
static tsfloat_t alt;
static sliding_avg_t *avg;

static i2c_dev_t device;
static bmp085_ctx_t context;


static float pressure_2_alt(float pressure, float start_pressure)
{
   return 44330.75 * (1.0 - powf(pressure / start_pressure, 0.19029));
}


SIMPLE_THREAD_BEGIN(thread_func)
{
   bmp085_read_temperature(&device, &context);
   float start_pressure = bmp085_read_pressure(&device, &context);

   SIMPLE_THREAD_LOOP_BEGIN
   {
      bmp085_read_temperature(&device, &context);
      float pressure = bmp085_read_pressure(&device, &context);
      float _alt = pressure_2_alt(pressure, start_pressure);
      _alt = sliding_avg_calc(avg, _alt);
      tsfloat_set(&alt, _alt);
      msleep(1);
   }
   SIMPLE_THREAD_LOOP_END
}
SIMPLE_THREAD_END


float bmp085_reader_get_alt(void)
{
   return tsfloat_get(&alt);
}


int bmp085_reader_init(i2c_bus_t *bus)
{
   ASSERT_ONCE();

   i2c_dev_init(&device, bus, "bmp085", 0x77);
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


