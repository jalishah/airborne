
/*
   DROTEK MARG2 platform driver - implementation

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


#include <util.h>


#include "drotek_marg2.h"


int drotek_marg2_init(drotek_marg2_t *marg2, i2c_bus_t *bus)
{
   THROW_BEGIN();
   /*
    * MPU6050_DLPF_CFG_260_256Hz,
      MPU6050_DLPF_CFG_184_188Hz,
      PU6050_DLPF_CFG_94_98Hz,
      MPU6050_DLPF_CFG_44_42Hz */
   THROW_ON_ERR(mpu6050_init(&marg2->mpu, bus, MPU6050_DLPF_CFG_260_256Hz, MPU6050_FS_SEL_500, MPU6050_AFS_SEL_4G));
   THROW_ON_ERR(hmc5883_init(&marg2->hmc, bus));
   THROW_END();
}


int drotek_marg2_read(marg_data_t *data, drotek_marg2_t *marg2)
{
   THROW_BEGIN();
   THROW_ON_ERR(mpu6050_read(&marg2->mpu, &data->gyro, &data->acc, NULL));
   THROW_ON_ERR(hmc5883_read_mag(data->mag.vec, &marg2->hmc));
   THROW_END();
}


