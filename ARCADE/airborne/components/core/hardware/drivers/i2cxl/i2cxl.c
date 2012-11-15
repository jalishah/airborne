
/*
   MaxSonar I2C Linux Userspace Driver

   Copyright (C) Tobias Simon, Ilmenau University of Technology

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
*/


#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#include "i2cxl.h"


#define I2CXL_ADDRESS       0xE0
#define I2CXL_RANGE_COMMAND 0x51
#define I2CXL_READ          0x02


THROW i2cxl_init(i2cxl_t *i2cxl, i2c_bus_t *bus)
{
   THROW_BEGIN();

   /* copy values */
   i2c_dev_init(&i2cxl->i2c_dev, bus, I2CXL_ADDRESS);

   THROW_END();
}


THROW i2cxl_read(i2cxl_t *i2cxl, float *dist)
{
   THROW_BEGIN();
   
   THROW_ON_ERR(i2c_write(&i2cxl->i2c_dev, I2CXL_RANGE_COMMAND));

   msleep(100);
   
   uint8_t raw[2];
   THROW_ON_ERR(i2c_read_block_reg(&i2cxl->i2c_dev, I2CXL_READ, raw, sizeof(raw)));

   THROW_END();
}

