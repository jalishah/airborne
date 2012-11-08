
/*
 * rc.h
 *
 * purpose: remote control interface
 * author: Tobias Simon, Ilmenau University of Technology
 */


#ifndef __RC_H__
#define __RC_H__


#define MAX_CHANNELS 5


typedef enum
{
   CH_PITCH,
   CH_ROLL,
   CH_YAW,
   CH_GAS,
   CH_KILL
}
channel_t;



/*
 * remote control device
 */
typedef struct
{
   int (*read)(float channels[MAX_CHANNELS]);
}
rc_interface_t;


/*
 * creates an rc interface
 */
rc_interface_t *rc_interface_create(int (*read)(float channels[MAX_CHANNELS]));


int rc_read(rc_interface_t *interface, float channels[MAX_CHANNELS]);


#endif /* __RC_H__ */

