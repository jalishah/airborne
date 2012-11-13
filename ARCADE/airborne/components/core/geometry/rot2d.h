
/*
   2d rotations - interface

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


#ifndef ROT2D_H
#define ROT2D_H


/**
 * this context has to be initialized by rot2d_impl_t.init
 * after initialization, rot2d_impl_t.calc may be used to calculate the rotation
 */
struct rot2d_context;
typedef struct rot2d_context rot2d_context_t;


rot2d_context_t *rot2d_create(void);

void rot2d_calc(rot2d_context_t *context, float out[2], const float in[2], float angle);


#endif /* ROT2D_H */
