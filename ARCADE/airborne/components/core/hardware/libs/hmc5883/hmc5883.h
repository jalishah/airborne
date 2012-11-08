
/*
   HMC5883 I2C Linux Userspace Driver

   Copyright (C) 2012 Tobias Simon

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
*/


#ifndef __HMC5883_H__
#define __HMC5883_H__


#include <stdint.h>

#include <util.h>

#include "../../bus/i2c/i2c.h"
#include "../../../geometry/orientation.h"


typedef struct
{
   /* i2c device: */
   i2c_dev_t i2c_dev;
}
hmc5883_t;


THROW hmc5883_init(hmc5883_t *dev, i2c_bus_t *bus);

THROW hmc5883_read_mag(float mag[3], hmc5883_t *dev);


#endif /* __HMC5883_H__ */

