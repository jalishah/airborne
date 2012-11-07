

/*
   BMA180 I2C Linux Userspace Driver

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



#ifndef __MK_BLMC_H__
#define __MK_BLMC_H__


#include <stdint.h>

#include "../../i2c/i2c.h"


typedef struct
{
   /* i2c device: */
   i2c_dev_t i2c_dev;
}
mk_blmc_dev_t;


int mk_blmc_init(mk_blmc_dev_t *dev, uint8_t addr, i2c_bus_t *bus);

int mk_blmc_write(mk_blmc_dev_t *dev, uint8_t val);


#endif /* __MK_BLMC_H__ */

