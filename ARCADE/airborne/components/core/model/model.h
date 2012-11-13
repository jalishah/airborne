
/*
   Position/Speed Model Interface

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


#ifndef __MODEL_H__
#define __MODEL_H__


#include "../geometry/orientation.h"


typedef struct
{
   float pos; /* position, in m */
   float speed; /* in m / s */
}
position_state_t;


typedef struct
{
   position_state_t x; /* x state */
   position_state_t y; /* y state */
   position_state_t ultra_z; /* ultrasonoc altitude over ground */
   position_state_t baro_z; /* barometric altitude above sea level */
}
model_state_t;


typedef struct
{
   float dt;

   /* positions input: */
   float ultra_z;
   float baro_z;
   float dx;
   float dy;

   /* control acc input: */
   vec3_t acc;
}
model_input_t;


void model_init(void);

void model_step(model_state_t *out, model_input_t *input);


#endif /* __MODEL_H__ */

