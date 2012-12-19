
/*
   ARCADE GPS Data Structure

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



#include <string.h>

#include "gps_data.h"


void gps_data_init(gps_data_t *gps_data)
{
   memset(&gps_data, 0, sizeof(gps_data));
}

