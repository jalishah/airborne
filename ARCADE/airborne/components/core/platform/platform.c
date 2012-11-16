
/*
   multirotor platform - implementation

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


#include "platform.h"

#include <string.h>
#include <malloc.h>
#include <assert.h>
#include <errno.h>


static platform_t platform;


int platform_init(int (*plat_init)(platform_t *platform))
{
   memset(&platform, 0, sizeof(platform));
   return plat_init(&platform);
}


#define CHECK_DEV(x) \
   if (!x) \
      return -ENODEV


int platform_read_marg(marg_data_t *marg_data)
{
   CHECK_DEV(platform.read_marg);
   return platform.read_marg(marg_data);
}


int platform_read_rc(float channels[MAX_CHANNELS])
{
   CHECK_DEV(platform.read_rc);
   return platform.read_rc(channels);
}


int platform_read_gps(gps_data_t *gps_data)
{
   CHECK_DEV(platform.read_gps);
   return platform.read_gps(gps_data);
}


int platform_read_ultra(float *ultra)
{
   CHECK_DEV(platform.read_ultra);
   return platform.read_ultra(ultra);
}


int platform_read_baro(float *baro)
{
   CHECK_DEV(platform.read_baro);
   return platform.read_baro(baro);
}


int platform_read_voltage(float *voltage)
{
   CHECK_DEV(platform.read_voltage);
   return platform.read_voltage(voltage);
}


int platform_write_motors(int enabled, float forces[4], float voltage)
{
   CHECK_DEV(platform.write_motors);
   return platform.write_motors(enabled, forces, voltage);
}


float platform_thrust(void)
{
   return platform.thrust;
}

