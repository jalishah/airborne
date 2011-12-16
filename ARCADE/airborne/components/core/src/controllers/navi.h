
#ifndef NAVIGATOR_H
#define NAVIGATOR_H


typedef struct
{
   float dest_x; /* destination in lon direction, in m (this also true for the fields below) */
   float dest_y; /* destination in lat direction */
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
void navigator_init(void);


/*
 * resets i-part(s) of the navigation algorithm
 */
void navigator_reset(void);


void navigator_reset_travel_speed(void);


int navigator_set_travel_speed(float speed);


/*
 * executes navigation control subsystem
 */
void navigator_run(navi_output_t *output, const navi_input_t *input);



#endif /* NAVIGATOR_H */

