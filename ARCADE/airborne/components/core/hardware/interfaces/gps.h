
/*
 * gps.h
 *
 *  Created on: 11.06.2010
 *      Author: tobi
 */


#ifndef __GPS_H__
#define __GPS_H__


typedef struct
{

   enum
   {
      FIX_NOT_SEEN, /* no mode available so far */
      FIX_NONE,     /* all fields are invalid */
      FIX_2D,       /* alt field is invalid */
      FIX_3D        /* all fields are valid */
   } fix;

   int satellites; /* number of satellites */

   /* start position */
   double start_lat;
   double start_lon;
   double start_alt;

   /* current position */
   double lat;
   double lon;
   double alt; /* above sea level, in m */

   /* deltas in m */
   double delta_x;
   double delta_y;
   double delta_z;

   float yaw; /* in rad */
   float ground_speed; /* in m/s */
   float climb_speed; /* in m/s */
}
gps_data_t;


#define GPS_DATA_INITIALIZER {FIX_NOT_SEEN, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}


typedef struct
{
   int (*init)(void);
   void (*read)(gps_data_t *data);
   float covar;
}
gps_interface_t;


extern double gps_start_coord[3];


int gps_init(gps_interface_t *interface);


void gps_read(gps_interface_t *interface, gps_data_t *data);


#endif /* __GPS_H__ */

