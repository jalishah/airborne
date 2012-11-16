
/*
   MaxSonar I2CXL I2C Linux Userspace Driver

   Copyright (C) 2012 Tobias Simon, Ilmenau University of Technology

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
*/


#ifndef __I2CXL_H__
#define __I2CXL_H__


#include <util.h>

#include "../../bus/i2c/i2c.h"


typedef struct
{
   /* i2c device: */
   i2c_dev_t i2c_dev;
}
i2cxl_t;


int i2cxl_init(i2cxl_t *i2cxl, i2c_bus_t *bus);

int i2cxl_read(i2cxl_t *i2cxl, float *dist);


#endif /* __I2CXL_H__ */

