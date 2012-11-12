
/*
   feed forward system and stabilizing PIID controller - interface

   Copyright (C) 2012 Alexander Barth, Ilmenau University of Technology
   Copyright (C) 2012 Benjamin Jahn, Ilmenau University of Technology
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


#ifndef __PIID_H__
#define __PIID_H__


#include "control_param.h"
#include "../util/adams4.h"
#include "../../filters/filter.h"


typedef struct 
{
   float Ts;

   float *f_local;
   float *xi_err;
   float *xii_err;

   adams4_t *int_err1;
   adams4_t *int_err2;

   unsigned int int_enable;

   Filter1 *filter_lp_err;
   Filter1 *filter_hp_err;

   Filter2 *filter_ref;
   Filter2 *filter_feedforw_x;
   Filter2 *filter_feedforw_y;
   Filter2 *filter_feedforw_z;

   float ringbuf[3 * CTRL_NUM_TSTEP];
   int ringbuf_idx;

   float test_out[3];
}
piid_t;


int piid_init(piid_t *ctrl, float sample_time);

void piid_run(piid_t *ctrl, float gyro[3], float rc[4], float u_ctrl[3]);

void piid_term(piid_t *ctrl);


#endif /* __PIID_H__ */

