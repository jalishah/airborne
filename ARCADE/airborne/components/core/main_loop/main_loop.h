
#ifndef __MAIN_LOOP_H__
#define __MAIN_LOOP_H__

#include <stdint.h>
#include "../platform/platform.h"


#define REALTIME_PERIOD (0.007)


#define DATA_DEFINITION() \
   float channels[MAX_CHANNELS]; \
   marg_data_t marg_data; \
   float dt; \
   float ultra_z; \
   float baro_z; \
   float voltage; \
   gps_data_t gps_data


void main_init(int override_hw);

void main_step(float dt, marg_data_t *marg_data, gps_data_t *gps_data, float ultra, float baro, float voltage, float channels[MAX_CHANNELS], uint16_t sensor_status, int override_hw);

void main_calibrate(int enabled);

#endif /* __MAIN_LOOP_H__ */

