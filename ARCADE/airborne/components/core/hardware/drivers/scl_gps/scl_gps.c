
#include <util.h>
#include <simple_thread.h>
#include <gps_data.pb-c.h>
#include <sclhelper.h>
#include "../../../util/time/interval.h"

#include "scl_gps.h"



static gps_data_t gps_data = {FIX_NOT_SEEN, 0, 0, 0, 0};
static simple_thread_t thread;
static void *scl_socket;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static int ret_code = 0;


SIMPLE_THREAD_BEGIN(thread_func)
{
   interval_t interval;
   interval_init(&interval);
   SIMPLE_THREAD_LOOP_BEGIN
   {
      GpsData *_gps_data;
      SCL_RECV_AND_UNPACK_DYNAMIC(_gps_data, scl_socket, gps_data);
      if (_gps_data != NULL)
      {
         pthread_mutex_lock(&mutex);
         if (interval_measure(&interval) > 2.0f)
         {
            ret_code = -EAGAIN;
         }
         gps_data.fix = _gps_data->fix;
         gps_data.sats = _gps_data->sats;
         gps_data.lat = _gps_data->lat;
         gps_data.lon = _gps_data->lon;
         gps_data.alt = _gps_data->alt;
         pthread_mutex_unlock(&mutex);
         SCL_FREE(gps_data, _gps_data);
      }
   }
   SIMPLE_THREAD_LOOP_END
}
SIMPLE_THREAD_END


int scl_gps_init(void)
{
   scl_socket = scl_get_socket("gps");
   if (scl_socket == NULL)
   {
      return -1;
   }
   simple_thread_start(&thread, thread_func, "gps_reader", 0, NULL);
   return 0;
}


int scl_gps_read(gps_data_t *data_out)
{
   pthread_mutex_lock(&mutex);
   *data_out = gps_data;
   int _ret_code = ret_code;
   pthread_mutex_unlock(&mutex);
   return _ret_code;
}

