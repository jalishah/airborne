
/*
   DROTEK MARG2 platform driver - interface

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


#ifndef __DROTEK_MARG2_H__
#define __DROTEK_MARG2_H__


#include <util.h>

#include "platform.h"

#include "../hardware/drivers/mpu6050/mpu6050.h"
#include "../hardware/drivers/hmc5883/hmc5883.h"


typedef struct
{
   mpu6050_t mpu;
   hmc5883_t hmc;
}
drotek_marg2_t;


int drotek_marg2_init(drotek_marg2_t *marg, i2c_bus_t *bus);

int drotek_marg2_read(marg_data_t *data, drotek_marg2_t *marg);


#endif /* __DROTEK_MARG2_H__ */

