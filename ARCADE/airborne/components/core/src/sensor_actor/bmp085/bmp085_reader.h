/*
 * srfx.h
 *
 * created on: 11.06.2011
 * author: tobi
 */

#ifndef BMP085_H
#define BMP085_H


#include "../../interfaces/altimeter.h"
#include "../../lib/i2c/interface/i2c_generic.h"


int bmp085_initialize(void);

int bmp085_read(alt_t *data_out);

int bmp085_finalize(void);


#endif /* BMP085_H */

