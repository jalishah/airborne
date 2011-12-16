
#include <pthread.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "scl_gps.h"
#include "util.h"
#include "gps_data.pb-c.h"
#include "../../util/types.h"
#include "../../util/logger/logger.h"
#include "../../util/threads/simple_thread.h"
#include "../../../../../../../common/scl/src/sclhelper.h"
#include "../../util/time/ltime.h"


#define GPS_THREAD_NAME       "gps-reader"
#define GPS_THREAD_PRIORITY   0


static gps_data_t gps_input_data = GPS_DATA_INITIALIZER;
static simple_thread_t thread;
static void *socket;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


SIMPLE_THREAD_BEGIN(gps_reader_thread_func)
{
   SIMPLE_THREAD_LOOP_BEGIN
   {
      unsigned char raw_data[1024];
      int raw_data_size = scl_recv_static(socket, raw_data, sizeof(raw_data));
      GpsData *gps_data = gps_data__unpack(NULL, raw_data_size, raw_data);
      if (gps_data == NULL)
      {
         continue;
      }

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
      gps_input_data.ground_speed = 0.0;
      gps_input_data.climb_speed = 0.0;
      
      pthread_mutex_unlock(&mutex);

      gps_data__free_unpacked(gps_data, NULL);
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

   simple_thread_start(&thread, gps_reader_thread_func,
                       GPS_THREAD_NAME, GPS_THREAD_PRIORITY, NULL);

   gps_input_data.fix = FIX_NOT_SEEN;
   while (1)
   {
      sleep(1);
      if (gps_input_data.fix == FIX_3D)
      {
         break;
      }
      LOG(LL_INFO, "waiting for 3d fix, current is: %d", gps_input_data.fix);
   }

   gps_input_data.start_lat = gps_input_data.lat;
   gps_input_data.start_lon = gps_input_data.lon;
   gps_input_data.start_alt = gps_input_data.alt;

   LOG(LL_INFO, "we have a fix: %f, %f, %f", gps_input_data.start_lat, gps_input_data.start_lon, gps_input_data.start_alt);
   return 0;
}


void scl_gps_read(gps_data_t *data_out)
{
   ASSERT_NOT_NULL(data_out);
   pthread_mutex_lock(&mutex);
   *data_out = gps_input_data;
   pthread_mutex_unlock(&mutex);
}

