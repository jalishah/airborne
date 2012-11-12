
#include "gps_util.h"
#include "../../geometry/earth.h"


void gps_util_init(gps_util_t *gps_util)
{
   gps_util->initialized = 0;   
}


void gps_util_update(gps_rel_data_t *out, gps_util_t *gps_util, gps_data_t *in)
{
   if (in->fix == FIX_3D)
   {
      if (!gps_util->initialized)
      {
         /* set-up start positions */
         gps_util->initialized = 1;
         gps_util->start_lon = in->lon;
         gps_util->start_lat = in->lat;
         gps_util->start_alt = in->alt;
      }
   }

   /* calculate deltas: */
   if (in->fix >= FIX_2D && gps_util->initialized)
   {
      meter_offset(&out->dx, &out->dy, in->lat, in->lon, gps_util->start_lat, gps_util->start_lon);
      if (in->fix == FIX_3D)
      {
         out->dz = in->alt - gps_util->start_alt;
      }
   }
}

