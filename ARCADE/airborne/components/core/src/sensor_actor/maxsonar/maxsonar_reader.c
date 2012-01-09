

#include "maxsonar_reader.h"
#include "maxsonar.h"

#include "util.h"
#include "../../../../../common/util/serial/serial.h"
#include "../../util/threads/simple_thread.h"
#include "../../util/threads/threadsafe_types.h"
#include "../../algorithms/median_filter.h"


#define THREAD_NAME       "maxsonar"
#define THREAD_PRIORITY   0

//#define USE_FILTER


static simple_thread_t thread;


static serialport_t port;
static maxsonar_t *sonar = NULL;

static threadsafe_float_t altitude;


SIMPLE_THREAD_BEGIN(thread_func)
{
#ifdef USE_FILTER
   const int WINDOW_SIZE = 5;
   float history[WINDOW_SIZE];
   float sorted[WINDOW_SIZE];
   median_filter_t filter;
   median_filter_init(&filter, history, sorted, WINDOW_SIZE);
#endif
   SIMPLE_THREAD_LOOP_BEGIN
   {
      uint8_t b = serial_read_char(&port);
      int status = maxsonar_parse(sonar, b);
      if (status == 1)
      {
#ifdef USE_FILTER
         threadsafe_float_set(&altitude, median_filter_run(&filter, maxsonar_get_dist(sonar)));
#else
         threadsafe_float_set(&altitude, maxsonar_get_dist(sonar));
#endif
      }
   }
   SIMPLE_THREAD_LOOP_END
}
SIMPLE_THREAD_END


int maxsonar_reader_init(void)
{
   ASSERT_ONCE();
   int status = serial_open(&port, "/dev/ttyO0", B9600, 0, 0, 0);
   if (status != 0)
   {
      return status;   
   }
   threadsafe_float_init(&altitude, 0.3);
   sonar = maxsonar_create();
   simple_thread_start(&thread, thread_func, THREAD_NAME, THREAD_PRIORITY, NULL);
   return 0;
}


float maxsonar_reader_get_alt(void)
{
   return threadsafe_float_get(&altitude);
}

