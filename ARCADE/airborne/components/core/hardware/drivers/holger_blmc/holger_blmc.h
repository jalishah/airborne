
/*
 * file: holger_blmc.c
 * purpose: mikrokopter brushless motor controller interface
 * author: Tobias Simon, Ilmenau University of Technology
 */


#ifndef __HOLGER_BLMC_H__
#define __HOLGER_BLMC_H__


#include <stdint.h>

#include "../../bus/i2c/i2c.h"


#define HOLGER_I2C_OFF    0
#define HOLGER_I2C_MIN   20
#define HOLGER_I2C_MAX  200



/*
 * creates a holger BLMC interface
 */
void holger_blmc_init(i2c_bus_t *bus, const uint8_t *addrs, const unsigned int n_motors);


/*
 * writes holger BLMC setpoints and reads rpm
 */
void holger_blmc_write_read(uint8_t *setpoints, uint8_t *rpm);


#endif /* __HOLGER_BLMC_H__ */

