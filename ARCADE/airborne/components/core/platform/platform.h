
/*
 * platform singleton
 */


#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include "../hardware/interfaces/gps.h"
#include "../geometry/orientation.h"
#include "../model/marg_data.h"

#if 0
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
#endif

#define MAX_CHANNELS 5


typedef enum
{
   CH_PITCH,
   CH_ROLL,
   CH_YAW,
   CH_GAS,
   CH_KILL
}
channel_t;


typedef struct
{
   /* sensors: */
   int (*read_marg)(marg_data_t *marg_data);
   int (*read_rc)(float channels[MAX_CHANNELS]);
   int (*read_gps)(gps_data_t *gps_data);
   int (*read_ultra)(float *ultra);
   int (*read_baro)(float *baro);
   int (*read_voltage)(float *voltage);
   
   /* actuators: */
   int (*write_motors)(int enabled, float forces[3], float voltage);
}
platform_t;



int platform_init(int (*plat_init)(platform_t *platform));


int platform_read_marg(marg_data_t *marg_data);


int platform_read_rc(float channels[MAX_CHANNELS]);


int platform_read_gps(gps_data_t *gps_data);


int platform_read_ultra(float *ultra);


int platform_read_baro(float *baro);


int platform_read_voltage(float *voltage);


int platform_write_motors(int enabled, float forces[4], float voltage);


#endif /* __PLATFORM_H__ */

