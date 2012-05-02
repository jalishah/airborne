
/*
 * File: z_ctrl.h
 * Type: PID controller
 * Purpose: altitude control using ground or MSL sensor input
 * Design Pattern: Singleton using z_ctrl_init
 *
 * Responsibilities:
 *   - self-configuration through OPCD
 *   - setpoint management
 *   - mode management (ground vs. MSL altitude)
 *   - controller state management
 *
 * Author: Tobias Simon, Ilmenau University of Technology
 */


#ifndef __Z_CTRL_H__
#define __Z_CTRL_H__


typedef struct
{
   float val;
   int mode_is_ground;
}
z_setpoint_t;



void z_ctrl_init(void);

void z_ctrl_set_setpoint(z_setpoint_t z_setpoint);

float z_ctrl_get_setpoint(void);

int z_ctrl_mode_is_ground(void);
 
float z_ctrl_step(float *z_err, float ground_z_pos, float z_pos, float speed, float dt);

void z_ctrl_reset(void);


#endif /* __ALT_CTRL_H__ */

