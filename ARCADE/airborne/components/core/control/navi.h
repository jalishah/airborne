
#ifndef NAVIGATOR_H
#define NAVIGATOR_H


typedef struct
{
   float pos_x; /* current position in lon direction */
   float pos_y; /* current position in lat direction */
   float speed_x; /* current speed in lon direction */
   float speed_y; /* current speed in lat direction */
   float acc_x;
   float acc_y;
   float dt; /* control loop iteration time delta, in seconds */
   float yaw; /* angle between magnetic north and device front, in rad */
}
navi_input_t;


typedef struct
{
   float pitch; /* desired pitch angle (implies certain acceleration, depending on gas value) */
   float roll; /* desired roll angle ("-") */
}
navi_output_t;


/*
 * allocates and initializes memory for navigation control subsystem
 */
void navi_init(void);


/*
 * resets i-part(s) of the navigation algorithm
 */
void navi_reset(void);


/*
 * setter for x position
 */
void navi_set_dest_x(float x);


/*
 * setter for y position
 */
void navi_set_dest_y(float y);


/*
 * getter for x position
 */
float navi_get_dest_x(void);


/*
 * getter for y position
 */
float navi_get_dest_y(void);


/*
 * set travel speed to standard
 */
void navi_reset_travel_speed(void);


/*
 * set navi travel speed
 */
int navi_set_travel_speed(float speed);


/*
 * executes navigation control subsystem
 */
void navi_run(navi_output_t *output, const navi_input_t *input);



#endif /* NAVIGATOR_H */

