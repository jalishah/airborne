
/*
 * gps_util.h
 *
 *  Created on: 11.06.2010
 *      Author: tobi
 */


#ifndef __GPS_UTIL_H__
#define __GPS_UTIL_H__


#include "gps_data.h"


typedef struct
{
   int initialized;
   double start_lon; /* start longitude in meters */
   double start_lat; /* start latitude in meters */
   double start_alt; /* start altitude in meters from MSL */
}
gps_util_t;


typedef struct
{
   double dx; /* delta x in meters */
   double dy; /* delta y in meters */
   double dz; /* delty z in meters */
}
gps_rel_data_t;


void gps_util_init(gps_util_t *gps_util);

void gps_util_update(gps_rel_data_t *out, gps_util_t *gps_util, gps_data_t *in);


#endif /* __GPS_UTIL_H__ */

