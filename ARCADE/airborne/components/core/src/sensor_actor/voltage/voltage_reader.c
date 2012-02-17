
#include <stdio.h>
#include <unistd.h>

#include <util.h>
#include <serial.h>
#include <simple_thread.h>
#include <threadsafe_types.h>
#include <opcd_params.h>


#include "voltage_reader.h"


#define THREAD_NAME       "voltage"
#define THREAD_PRIORITY   0


static simple_thread_t thread;
static char *sysfs_path = NULL;
static tsfloat_t voltage;


SIMPLE_THREAD_BEGIN(thread_func)
{
   SIMPLE_THREAD_LOOP_BEGIN
   {
      FILE *file = fopen(sysfs_path, "r");
      if (file != NULL)
      {
         int val;
         fscanf(file, "%d", &val);
         fclose(file);
         tsfloat_set(&voltage, ((float)val - 56.0) / 134.0);
      }
      sleep(1);
   }
   SIMPLE_THREAD_LOOP_END
}
SIMPLE_THREAD_END


float voltage_reader_get(void)
{
   return tsfloat_get(&voltage);   
}


void voltage_reader_start(void)
{
   opcd_param_t params[] =
   {
      {"sysfs_path", &sysfs_path},
      OPCD_PARAMS_END
   };
   opcd_params_apply("sensors.voltage.", params);
   tsfloat_init(&voltage, 16.0);

   simple_thread_start(&thread, thread_func, THREAD_NAME, THREAD_PRIORITY, NULL);
}

