
#include <util.h>
#include <simple_thread.h>
#include <gps_data.pb-c.h>
#include <sclhelper.h>

#include "scl_gps.h"
#include "../../../geometry/earth.h"


static gps_data_t gps_input_data = GPS_DATA_INITIALIZER;
static simple_thread_t thread;
static void *socket;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


SIMPLE_THREAD_BEGIN(thread_func)
{
   SIMPLE_THREAD_LOOP_BEGIN
   {
      GpsData *gps_data;
      SCL_RECV_AND_UNPACK_DYNAMIC(gps_data, socket, gps_data);
      if (gps_data != NULL)
      {
         pthread_mutex_lock(&mutex);
         gps_input_data.fix = gps_data->fix;
         gps_input_data.satellites = gps_data->sats;
         gps_input_data.lat = gps_data->lat;
         gps_input_data.lon = gps_data->lon;
         gps_input_data.alt = gps_data->alt;

         meter_offset(&gps_input_data.delta_x,
            &gps_input_data.delta_y,
            gps_input_data.lat,
            gps_input_data.lon,
            gps_input_data.start_lat,
            gps_input_data.start_lon);

         gps_input_data.delta_z = gps_input_data.alt - gps_input_data.start_alt;
         gps_input_data.ground_speed = gps_data->speed;
         gps_input_data.climb_speed = 0.0;
         pthread_mutex_unlock(&mutex);

         SCL_FREE(gps_data, gps_data);
      }
   }
   SIMPLE_THREAD_LOOP_END
}
SIMPLE_THREAD_END


int scl_gps_init(void)
{
   socket = scl_get_socket("gps");
   if (socket == NULL)
   {
      return -1;
   }
   simple_thread_start(&thread, thread_func, "gps_reader", 0, NULL);
   return 0;
}


int scl_gps_read(gps_data_t *data_out)
{
   pthread_mutex_lock(&mutex);
   *data_out = gps_input_data;
   pthread_mutex_unlock(&mutex);
   return 0; /* TODO: something more sophisticated required here */
}

