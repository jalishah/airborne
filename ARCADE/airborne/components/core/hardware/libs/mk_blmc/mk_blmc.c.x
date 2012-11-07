
/*
   Mikrokopter BLMC I2C Linux Userspace Driver

   Copyright (C) 2012 Jan Roemisch and Tobias Simon

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
*/


#include <stdio.h>
#include <string.h>

#include "mk_blmc.h"


int mk_blmc_init(mk_blmc_dev_t *dev, uint8_t addr, i2c_bus_t *bus)
{
   i2c_dev_init(&dev->i2c_dev, bus, addr);
   return 0;
}


int mk_blmc_write(mk_blmc_dev_t *dev, uint8_t val)
{
   return i2c_write(&dev->i2c_dev, val);
}

