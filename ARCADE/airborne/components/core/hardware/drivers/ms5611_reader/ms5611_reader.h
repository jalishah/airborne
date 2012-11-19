
#ifndef __MS5611_READER_H__
#define __MS5611_READER_H__


#include "../../bus/i2c/i2c.h"


int ms5611_reader_init(i2c_bus_t *bus);

int ms5611_reader_get_alt(float *alt);


#endif /* __MS5611_READER_H__ */

