

#ifndef __SCL_GPS_H__
#define __SCL_GPS_H__


#include "../../util/gps_data.h"


int scl_gps_init(void);

int scl_gps_read(gps_data_t *data);


#endif /* __SCL_GPS_H__ */

