
/*
   system and control parameters - interface

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


#ifndef __CONTROL_PARAM_H__
#define __CONTROL_PARAM_H__


/* ----- system parameter ----- */
#define CTRL_L 0.2025f

#define CTRL_JXX 1.0f * 0.0097f
#define CTRL_JYY 1.0f * 0.0097f
#define CTRL_JZZ 0.45f * 1.273177e-002f

#define CTRL_TMC 0.06f


/* ----- parameter for input compensation ----- */
/* rpm^2 =a*voltage^1.5*i2c^b */
#define CTRL_F_A 609.6137f
#define CTRL_F_B 1.3154f

/* F = c*rpm^2 */
#define CTRL_F_C 1.5866e-007f

/* tau = d*rpm^2 */
#define CTRL_F_D 4.0174e-009f



/* ----- control parameter ----- */
#define CTRL_NUM_TSTEP 7

/* Pitch/Roll: */
#define PIID_KD  0.02352953418f
#define PIID_KP  0.32876523028f
#define PIID_KI  2.20026754887f
#define PIID_KII 4.37086296837f

/* Yaw: */
#define PIID_Y_KP  0.108f
#define PIID_Y_KD  0.00648f
#define PIID_Y_KI  0.45f
#define PIID_Y_KII 0.0f

/* Filter Parameter Feed Forward */
#define FILT_FF_FG 10.0f
#define FILT_FF_D  0.95f

/* Filter Parameter Controller */
#define FILT_C_FG 10.0f


#endif /* __SYS_PARAM_H__ */
