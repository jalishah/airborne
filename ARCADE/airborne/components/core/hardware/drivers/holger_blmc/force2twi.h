
#ifndef __FORCE2TWI_H__
#define __FORCE2TWI_H__


#include <stdint.h>


int force2twi_calc(uint8_t *i2c, const float voltage, const float *rpm_square, const size_t n_motors);


#endif /* __FORCE2TWI_H__ */

