
/*
   Feed Forward System Implementation

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


#include <math.h>
#include <util.h>

#include "feed_forward.h"
#include "../../filters/filter.h"


/* system parameters: */
#define CTRL_JXX (1.0f * 0.0097f)
#define CTRL_JYY (1.0f * 0.0097f)
#define CTRL_JZZ (0.45f * 1.273177e-002f)
#define CTRL_TMC (0.06f)


void feed_forward_init(feed_forward_t *ff, float Ts)
{
   float T = 1.0f / (2.0f * M_PI * FILT_FF_FG);
   float temp_a0 = (4.0f * T * T + 4.0f * FILT_FF_D * T * Ts + Ts*Ts);

   float a[2] = {
      (2.0f * Ts * Ts - 8.0f * T * T) / temp_a0,
      (4.0f * T * T   - 4.0f * FILT_FF_D * T * Ts + Ts * Ts) / temp_a0
   };

   /* x-axis: */
   float b[3] = {
       (2.0f * CTRL_JXX * (2.0f * CTRL_TMC + Ts)) / temp_a0,
      -(8.0f * CTRL_JXX * CTRL_TMC) / temp_a0,
       (2.0f * CTRL_JXX * (2.0f * CTRL_TMC - Ts)) / temp_a0
   };
   filter2_init(&ff->filters[0], a, b, Ts, 1);

   /* y-axis: */
   b[0] =  (2.0f * CTRL_JYY * (2.0f * CTRL_TMC + Ts)) / temp_a0;
   b[1] = -(8.0f * CTRL_JYY * CTRL_TMC) / temp_a0;
   b[2] =  (2.0f * CTRL_JYY * (2.0f * CTRL_TMC - Ts)) / temp_a0;
   filter2_init(&ff->filters[1], a, b, Ts, 1);

   /* z-axis: */
   b[0] =  (2.0f * CTRL_JZZ * (2.0f * CTRL_TMC + Ts)) / temp_a0;
   b[1] = -(8.0f * CTRL_JZZ * CTRL_TMC) / temp_a0;
   b[2] =  (2.0f * CTRL_JZZ * (2 * CTRL_TMC - Ts)) / temp_a0;
   filter2_init(&ff->filters[2], a, b, Ts, 1);
}


void feed_forward_run(feed_forward_t *ff, float u_ctrl[3], float torques[3])
{
   FOR_N(i, 3)
   {
      filter2_run(&ff->filters[i], &torques[i], &u_ctrl[i]);
   }
}

