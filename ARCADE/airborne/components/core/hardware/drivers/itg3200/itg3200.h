
/*
   ITG3200 I2C Linux Userspace Driver

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


#ifndef __ITG3200_H__
#define __ITG3200_H__


#include <stdint.h>

#include <util.h>

#include "../../bus/i2c/i2c.h"
#include "../../../geometry/orientation.h"


/* low-pass filter options */
typedef enum
{
   ITG3200_DLPF_256HZ = 0,
   ITG3200_DLPF_188HZ,
   ITG3200_DLPF_98HZ,
   ITG3200_DLPF_42HZ,
   ITG3200_DLPF_20HZ,
   ITG3200_DLPF_10HZ,
   ITG3200_DLPF_5HZ,
} 
itg3200_dlpf_t;


typedef struct
{
   /* i2c device: */
   i2c_dev_t i2c_dev;

   /* filter configuration: */
   itg3200_dlpf_t lp_filter;
}
itg3200_t;


THROW itg3200_init(itg3200_t *dev, i2c_bus_t *bus, itg3200_dlpf_t filter);

THROW itg3200_zero_gyros(itg3200_t *dev);

THROW itg3200_read_gyro(float gyro[3], itg3200_t *dev);

THROW itg3200_read_temperature(float *temperature, itg3200_t *dev);


#endif /* __ITG3200_H__ */

