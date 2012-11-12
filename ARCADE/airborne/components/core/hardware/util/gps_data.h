
/*
 * gps.h
 *
 *  Created on: 11.06.2010
 *      Author: tobi
 */


#ifndef __GPS_DATA_H__
#define __GPS_DATA_H__


typedef struct
{
   enum
   {
      FIX_NOT_SEEN, /* no mode available so far */
      FIX_NONE,     /* all fields are invalid */
      FIX_2D,       /* alt field is invalid */
      FIX_3D        /* all fields are valid */
   }
   fix;

   int sats; /* number of satellites */
   double lon; /* longitude, east direction */
   double lat; /* latitude, north direction  */
   double alt; /* above sea level, in m */
}
gps_data_t;


#endif /* __GPS_DATA_H__ */

