

#include <serial.h>
#include <simple_thread.h>
#include <threadsafe_types.h>

#include "i2cxl_reader.h"
#include "../i2cxl/i2cxl.h"


#define THREAD_NAME       "i2cxl_reader"
#define THREAD_PRIORITY   0


static simple_thread_t thread;
static i2cxl_t i2cxl;
static tsfloat_t altitude;
static int status;


SIMPLE_THREAD_BEGIN(thread_func)
{
   SIMPLE_THREAD_LOOP_BEGIN
   {
      float alt;
      status = i2cxl_read(&i2cxl, &alt);
      if (status == 0)
      {
         tsfloat_set(&altitude, alt);
      }
      msleep(1);
   }
   SIMPLE_THREAD_LOOP_END
}
SIMPLE_THREAD_END


int i2cxl_reader_init(i2c_bus_t *bus)
{
   ASSERT_ONCE();
   tsfloat_init(&altitude, 0.2);
   i2cxl_init(&i2cxl, bus);
   simple_thread_start(&thread, thread_func, THREAD_NAME, THREAD_PRIORITY, NULL);
   return 0;
}


int i2cxl_reader_get_alt(float *alt)
{
   if (status == 0)
   {
      *alt = tsfloat_get(&altitude);
   }
   return status;
}

