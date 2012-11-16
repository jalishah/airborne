
/*
   multirotor platform - interface

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


#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include "../hardware/util/gps_data.h"
#include "../hardware/util/rc_channels.h"
#include "../hardware/util/marg_data.h"
#include "../geometry/orientation.h"


typedef struct
{
   /* parameters: */
   float thrust;

   /* sensors: */
   int (*read_marg)(marg_data_t *marg_data);
   int (*read_rc)(float channels[MAX_CHANNELS]);
   int (*read_gps)(gps_data_t *gps_data);
   int (*read_ultra)(float *ultra);
   int (*read_baro)(float *baro);
   int (*read_voltage)(float *voltage);
   
   /* actuators: */
   int (*write_motors)(int enabled, float forces[3], float voltage);
}
platform_t;



int platform_init(int (*plat_init)(platform_t *platform));


int platform_read_marg(marg_data_t *marg_data);


int platform_read_rc(float channels[MAX_CHANNELS]);


int platform_read_gps(gps_data_t *gps_data);


int platform_read_ultra(float *ultra);


int platform_read_baro(float *baro);


int platform_read_voltage(float *voltage);


int platform_write_motors(int enabled, float forces[4], float voltage);


float platform_thrust(void);


#endif /* __PLATFORM_H__ */

