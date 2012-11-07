
/*
   DROTEK MARG Linux Userspace Driver

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


#ifndef __DROTEK_MARG_H__
#define __DROTEK_MARG_H__


#include <util.h>


#include "../hardware/libs/itg3200/itg3200.h"
#include "../hardware/libs/bma180/bma180.h"
#include "../hardware/libs/hmc5883/hmc5883.h"


typedef struct
{
   vec3_t gyro;
   vec3_t acc;
   vec3_t mag;
}
marg_data_t;


typedef struct
{
   itg3200_t itg;
   bma180_t bma;
   hmc5883_t hmc;
}
drotek_marg_t;


THROW drotek_marg_init(drotek_marg_t *marg, i2c_bus_t *bus);

THROW drotek_marg_read(marg_data_t *data, drotek_marg_t *marg);


#endif /* __DROTEK_MARG_H__ */

