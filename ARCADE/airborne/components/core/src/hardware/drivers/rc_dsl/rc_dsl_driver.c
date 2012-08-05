
#include <util.h>
#include <serial.h>
#include <simple_thread.h>
#include <opcd_params.h>
#include "../../util/time/ltime.h"


#include "rc_dsl.h"


#define THREAD_NAME       "rc_dsl"
#define THREAD_PRIORITY   0


static simple_thread_t thread;


static serialport_t port;
static rc_dsl_t *rc_dsl = NULL;
static char *dev_path = NULL;
static rc_data_t data;


SIMPLE_THREAD_BEGIN(thread_func)
{
   SIMPLE_THREAD_LOOP_BEGIN
   {
      uint8_t b = serial_read_char(&port);
      if (rc_dsl_parse_dsl_data(rc_dsl, b) == 1)
      {
         
      }
   }
   SIMPLE_THREAD_LOOP_END
}
SIMPLE_THREAD_END


void rc_dsl_reader_start(void)
{
   memset(&data, 0, sizeof(rc_data_t));
   opcd_param_t params[] =
   {
      {"serial_port", &dev_path},
      OPCD_PARAMS_END   
   };
   opcd_params_apply("sensors.rc_dsl.", params);
   
   serial_open(&port, dev_path, 38400, 0, 0, 0);
   rc_dsl = rc_dsl_create();
   simple_thread_start(&thread, thread_func, THREAD_NAME, THREAD_PRIORITY, NULL);
}

