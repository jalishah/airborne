
#include "util.h"
#include "../../../../../common/util/serial/serial.h"
#include "../../util/threads/simple_thread.h"
#include "../../util/opcd_params/opcd_params.h"
#include "../../util/time/ltime.h"


#include "rc_dsl.h"


#define THREAD_NAME       "rc_dsl"
#define THREAD_PRIORITY   0


static simple_thread_t thread;


static serialport_t port;
static rc_dsl_t *rc_dsl = NULL;
static char *dev_path = NULL;


SIMPLE_THREAD_BEGIN(thread_func)
{
   SIMPLE_THREAD_LOOP_BEGIN
   {
      uint8_t b = serial_read_char(&port);
      rc_dsl_parse_dsl_data(rc_dsl, b);
   }
   SIMPLE_THREAD_LOOP_END
}
SIMPLE_THREAD_END


void rc_dsl_reader_start(void)
{
   opcd_param_t params[] =
   {
      {"serial_port", &dev_path},
      OPCD_PARAMS_END   
   };
   opcd_params_apply("sensors.rc_dsl.", params);
   
   serial_open(&port, dev_path, B38400, 0, 0, 0);
   rc_dsl = rc_dsl_create();
   simple_thread_start(&thread, thread_func, THREAD_NAME, THREAD_PRIORITY, NULL);
}

