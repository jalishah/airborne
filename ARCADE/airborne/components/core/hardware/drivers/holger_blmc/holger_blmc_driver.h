
/*
 * file: holger_blmc_driver.h
 * purpose: mikrokopter brushless motor controller driver
 * author: Tobias Simon, Ilmenau University of Technology
 */


#ifndef __HOLGER_BLMC_DRIVER_H__
#define __HOLGER_BLMC_DRIVER_H__


#include <stdint.h>
#include <stdbool.h>


#include "../../bus/i2c/i2c_generic.h"
#include "../../../platform/coupling.h"


#define HOLGER_I2C_OFF   0
#define HOLGER_I2C_MIN  20
#define HOLHER_I2C_MAX 255


/*
 * creates a holger BLMC interface
 */
void holger_blmc_driver_init(i2c_bus_t *bus, uint8_t *addrs, coupling_t *coupling, unsigned int n_motors);


/*
 * spins up the motors
 */
int holger_blmc_driver_start_motors(void);


/*
 * spins down the motors
 */
void holger_blmc_driver_stop_motors(void);


/*
 * writes holger BLMC setpoints
 */
bool holger_blmc_driver_write_forces(float forces[4], float voltage, float *rpm);



#endif /* __HOLGER_BLMC_DRIVER_H__ */

