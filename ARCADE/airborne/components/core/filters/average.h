
/*
   arithmetic average interface

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


#ifndef __AVERAGE_H__
#define __AVERAGE_H__


typedef struct
{
   float sum;
   float avg;
   int count;
   int max_count;
}
avg_data_t;


#define AVG_DATA_INITIALIZER(max) {0, 0, 0, max}

void avg_init(avg_data_t *avg_data, int max);

void avg_add(avg_data_t *avg_data, float value);


#endif /* __AVERAGE_H__ */

