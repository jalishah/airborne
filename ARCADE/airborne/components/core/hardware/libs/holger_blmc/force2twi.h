
#ifndef __FORCE2TWI_H__
#define __FORCE2TWI_H__


#include <stdbool.h>
#include <stdint.h>
#include "../../../platform/coupling.h"



void force2twi_init(const coupling_t *coup);

bool force2twi_calc(const float *force, const float voltage, uint8_t *i2c);


#endif /* __FORCE2TWI_H__ */

