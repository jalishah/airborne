

#ifndef __SCL_GPS_H__
#define __SCL_GPS_H__


#include "../interfaces/gps.h"


int scl_gps_init(void);

void scl_gps_read(gps_data_t *data);


#endif /* __SCL_GPS_H__ */

