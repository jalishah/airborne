

#include <unistd.h>

#include "gps.h"
#include "../../util/logger/logger.h"


double gps_start_coord[3];



int gps_init(gps_interface_t *interface)
{
   int status;
   if ((status = interface->init()) != 0)
   {
      goto out;   
   }

   gps_data_t data;
   /* wait for 3d fix: */
   while (1)
   {
      interface->read(&data);
      if (data.fix == FIX_3D)
      {
         break;
      }
      LOG(LL_INFO, "waiting for 3d fix, current is: %d", data.fix);
      sleep(5);
   }

   /* set starting position: */
   data.start_lat = data.lat;
   data.start_lon = data.lon;
   data.start_alt = data.alt;
   gps_start_coord[0] = data.start_lon;
   gps_start_coord[1] = data.start_lat;
   gps_start_coord[2] = data.start_alt;

   LOG(LL_INFO, "we have a fix: %f, %f, %f", data.start_lat, data.start_lon, data.start_alt);

out:
   return status;
}


void gps_read(gps_interface_t *interface, gps_data_t *data)
{
   interface->read(data);  
}



