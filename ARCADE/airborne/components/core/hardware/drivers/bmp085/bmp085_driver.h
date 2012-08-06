
/*
 * bmp085_driver.h
 *
 * created: 05.08.2012
 * author: Tobias Simon, Ilmenau University of Technology
 */


#ifndef __BMP085_DRIVER_H__
#define __BMP085_DRIVER_H__


#include "../../../hardware/bus/i2c/i2c_generic.h"


/*
 * initializes the driver
 */
int bmp085_driver_init(i2c_bus_t *bus);


/*
 * read altitude
 */
float bmp085_driver_read(void);


#endif /* __BMP085_DRIVER_H__ */

