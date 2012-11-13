
/*
   earth surface calculations - interface

   Copyright (C) 2012 Tobias Simon, Ilmenau University of Technology
   most of the code was taken from gpsd: http://gpsd.berlios.de

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
 */


#ifndef __EARTH_H__
#define __EARTH_H__


void meter_offset(double *dx, double *dy, double lat1, double lon1, double lat2, double lon2);


#endif /* __EARTH_H__ */

