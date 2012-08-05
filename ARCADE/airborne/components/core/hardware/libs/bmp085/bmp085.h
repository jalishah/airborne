
/*
 * bmp085.h
 *
 * created on: 10.05.2011
 * author: tobi
 */

#ifndef BMP085_H
#define BMP085_H


#include <time.h>
#include <stdint.h>
#include "../../bus/i2c/i2c_generic.h"


/*
 * bmp085 calibration data
 */
typedef struct
{
   int16_t AC1, AC2, AC3;
   uint16_t AC4, AC5, AC6;
   int16_t B1, B2;
   int16_t MB, MC, MD;
}
bmp085_cal_t;


/*
 * bmp085 context
 * - calibration data
 * - settings
 * - readings and timings
 */
typedef struct
{
   uint8_t version;
   bmp085_cal_t calibration;
   unsigned long raw_temperature;
   unsigned long raw_pressure;
   uint8_t oversampling_setting;
   time_t next_temp_measurement;
   long b6;
}
bmp085_ctx_t;


int bmp085_init(i2c_dev_t *dev, bmp085_ctx_t *ctx);

int bmp085_read_temperature(i2c_dev_t *dev, bmp085_ctx_t *ctx);

int bmp085_read_pressure(i2c_dev_t *dev, bmp085_ctx_t *ctx);


#endif /* BMP085_H */

