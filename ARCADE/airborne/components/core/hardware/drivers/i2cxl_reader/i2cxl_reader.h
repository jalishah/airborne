
#ifndef __I2CXL_READER_H__
#define __I2CXL_READER_H__


#include "../../bus/i2c/i2c.h"


int i2cxl_reader_init(i2c_bus_t *bus);

int i2cxl_reader_get_alt(float *alt);


#endif /* __I2CXL_READER_H__ */

