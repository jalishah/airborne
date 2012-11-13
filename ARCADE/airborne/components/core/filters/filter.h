
/*
   Filter Library Interface

   Copyright (C) 2012 Alexander Barth, Ilmenau University of Technology
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


#ifndef __FILTER_H__
#define __FILTER_H__


typedef struct 
{
    float Ts;
    int signal_dim;
    float *z;
    float a1;
    float b0;
    float b1;
}
Filter1;


typedef struct 
{
    float Ts;
    int signal_dim;
    float *z1;
    float *z2;
    float a1;
    float a2;
    float b0;
    float b1;
    float b2;
}
Filter2;


/* init functions */
void filter1_lp_init(Filter1 *filter, float fg, float Ts, int dim);

void filter1_hp_init(Filter1 *filter, float fg, float Ts, int dim);

void filter1_init(Filter1 *filter, float *a, float *b, float Ts, int dim);

void filter2_lp_init(Filter2 *filter, float fg, float damping, float Ts, int dim);

void filter2_hp_init(Filter2 *filter, float fg, float damping, float Ts, int dim);

void filter2_init(Filter2 *filter, float *a, float *b, float Ts, int dim);

/* execution functions */
void filter1_run(Filter1 *filter, float *u, float *y);

void filter2_run(Filter2 *filter, float *u, float *y);


#endif /* __FILTER_H__ */
