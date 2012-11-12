
/*
   feed forward system and stabilizing PIID controller - implementation

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


#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <util.h>

#include "piid.h"
#include "../../filters/filter.h"


static void ctrl_feed_forward(piid_t *piid, float rc[4])
{
   float temp_ff;

   filter2_run(piid->filter_feedforw_x, &rc[0], &temp_ff);
   piid->f_local[1] = temp_ff;

   filter2_run(piid->filter_feedforw_y, &rc[1], &temp_ff);
   piid->f_local[2] = temp_ff;

   filter2_run(piid->filter_feedforw_z, &rc[2], &temp_ff);
   piid->f_local[3] = temp_ff;
}


static void ctrl_feed_forward_init(piid_t *piid, float Ts)
{
   float T = 1.0f / (2.0f * M_PI * FILT_FF_FG);
   float temp_a0 = (4.0f * T * T + 4.0f * FILT_FF_D * T * Ts + Ts*Ts);

   float a[2] = {
       (2.0f * Ts * Ts - 8.0f * T * T) / temp_a0,
       (4.0f * T * T   - 4.0f * FILT_FF_D * T * Ts + Ts * Ts) / temp_a0
   };

   /* x-axis */
   float b[3] = {
       (2.0f * CTRL_JXX * (2.0f * CTRL_TMC + Ts)) / temp_a0,
      -(8.0f * CTRL_JXX * CTRL_TMC) / temp_a0,
       (2.0f * CTRL_JXX * (2.0f * CTRL_TMC - Ts)) / temp_a0
   };
   filter2_init(piid->filter_feedforw_x, a, b, Ts, 1);

   /* y-axis */
   b[0] =  (2.0f * CTRL_JYY * (2.0f * CTRL_TMC + Ts)) / temp_a0;
   b[1] = -(8.0f * CTRL_JYY * CTRL_TMC) / temp_a0;
   b[2] =  (2.0f * CTRL_JYY * (2.0f * CTRL_TMC - Ts)) / temp_a0;
   filter2_init(piid->filter_feedforw_y, a, b, Ts, 1);

   /* z-axis */
   b[0] =  (2.0f * CTRL_JZZ * (2.0f * CTRL_TMC + Ts)) / temp_a0;
   b[1] = -(8.0f * CTRL_JZZ * CTRL_TMC) / temp_a0;
   b[2] =  (2.0f * CTRL_JZZ * (2 * CTRL_TMC - Ts)) / temp_a0;
   filter2_init(piid->filter_feedforw_z, a, b, Ts, 1);
}


int piid_init(piid_t *piid, float Ts)
{
   piid->Ts = Ts;
   piid->int_enable = 1;
   piid->int_err1        = (adams4_t* )calloc(1,sizeof(adams4_t));
   piid->int_err2        = (adams4_t* )calloc(1,sizeof(adams4_t));
   piid->filter_lp_err   = (Filter1* )calloc(1,sizeof(Filter1));
   piid->filter_hp_err   = (Filter1* )calloc(1,sizeof(Filter1));
   piid->filter_ref        = (Filter2*)calloc(1,sizeof(Filter2));
   piid->filter_feedforw_x = (Filter2* )calloc(1,sizeof(Filter2));
   piid->filter_feedforw_y = (Filter2* )calloc(1,sizeof(Filter2));
   piid->filter_feedforw_z = (Filter2* )calloc(1,sizeof(Filter2));

   if (
           (piid->int_err1 == NULL)||
           (piid->int_err2 == NULL)||
           (piid->filter_lp_err == NULL)||
           (piid->filter_hp_err == NULL)||
           (piid->filter_ref == NULL)||
           (piid->filter_feedforw_x == NULL)||
           (piid->filter_feedforw_y == NULL)||
           (piid->filter_feedforw_z == NULL))
   {
      return 0;
   }
   if ((adams4_init(piid->int_err1,3)==0)||adams4_init(piid->int_err2,3)==0)
   {
      return 0;
   }

   filter1_lp_init(piid->filter_lp_err, FILT_C_FG, Ts, 3);
   filter1_hp_init(piid->filter_hp_err, FILT_C_FG, Ts, 3);
   filter2_lp_init(piid->filter_ref, FILT_FF_FG,FILT_FF_D, Ts, 3);
   ctrl_feed_forward_init(piid, Ts);

   piid->xi_err  = (float *)calloc(3,sizeof(float));
   piid->xii_err = (float *)calloc(3,sizeof(float));
   piid->f_local = (float *)calloc(4,sizeof(float));

   if ((piid->xi_err == NULL)||(piid->xii_err == NULL)||(piid->f_local == NULL))
   {
      return 0;
   }

   // init ring-buffer
   int i;
   for (i=0; i < 3*CTRL_NUM_TSTEP; i++)
   {
       piid->ringbuf[i] = 0;
   }
   piid->ringbuf_idx = 0;
   return 1;
}


void piid_run(piid_t *piid, float gyro[3], float rc[4], float u_ctrl[3])
{
    float error[3];
    float derror[3];

    /* feed forward calculation */
    ctrl_feed_forward(piid, rc);

    /* filter reference signals */
    filter2_run(piid->filter_ref, rc, rc);

    error[0] = piid->ringbuf[piid->ringbuf_idx + 0] - gyro[0];
    error[1] = piid->ringbuf[piid->ringbuf_idx + 1] - gyro[1];
    error[2] = piid->ringbuf[piid->ringbuf_idx + 2] - gyro[2];

    piid->ringbuf[piid->ringbuf_idx]   = rc[0];
    piid->ringbuf[piid->ringbuf_idx+1] = rc[1];
    piid->ringbuf[piid->ringbuf_idx+2] = rc[2];

    piid->ringbuf_idx += 3;
    if (piid->ringbuf_idx >= 3 * CTRL_NUM_TSTEP)
    {
        piid->ringbuf_idx = 0;
    }

    /* error bandpass filter: */
    filter1_run(piid->filter_hp_err, error, derror);
    filter1_run(piid->filter_lp_err, error, error);

    /* error integration: */
    piid->int_err1->f0[0] = error[0];
    piid->int_err1->f0[1] = error[1];
    piid->int_err1->f0[2] = error[2];
    adams4_run(piid->int_err1, piid->xi_err, piid->Ts, piid->int_enable);

    /* 2nd error integration: */
    piid->int_err2->f0[0] = piid->xi_err[0];
    piid->int_err2->f0[1] = piid->xi_err[1];
    piid->int_err2->f0[2] = piid->xi_err[2];
    adams4_run(piid->int_err2, piid->xii_err, piid->Ts, piid->int_enable);

    /* piid piid: */
    u_ctrl[0] = PIID_KP * error[0] + PIID_KI * piid->xi_err[0] + PIID_KII * piid->xii_err[0] + PIID_KD * derror[0];
    u_ctrl[1] = PIID_KP * error[1] + PIID_KI * piid->xi_err[1] + PIID_KII * piid->xii_err[1] + PIID_KD * derror[1];
    u_ctrl[2] = PIID_Y_KP * error[2] +  PIID_Y_KI * piid->xi_err[2] + PIID_Y_KII * piid->xii_err[2] +  PIID_Y_KD * derror[2];

    /* add piid output to feedforward: */
    piid->f_local[1] += u_ctrl[0];
    piid->f_local[2] += u_ctrl[1];
    piid->f_local[3] += u_ctrl[2];
}



void piid_piid_term(piid_t *piid)
{
   adams4_term(piid->int_err1);
   adams4_term(piid->int_err2);

   free(piid->xi_err);
   free(piid->f_local);
}

