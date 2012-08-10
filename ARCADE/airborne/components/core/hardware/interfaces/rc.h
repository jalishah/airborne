
/*
 * rc.h
 *
 * purpose: remote control interface
 * author: Tobias Simon, Ilmenau University of Technology
 */


#ifndef __RC_H__
#define __RC_H__


#define EXTRA_RC_CHANNELS (2)


/*
 * remote control data
 */
typedef struct
{
   float pitch;
   float roll;
   float yaw;
   float gas;
   float extra[EXTRA_RC_CHANNELS];
   float rssi;
}
rc_data_t;


/*
 * remote control device
 */
typedef struct
{
   int (*init)(void);
   void (*read)(rc_data_t *data);
}
rc_interface_t;


/*
 * creates an rc interface
 */
rc_interface_t *rc_interface_create(int (*init)(void), void (*read)(rc_data_t *data));


int rc_init(rc_interface_t *interface);

void rc_read(rc_interface_t *interface, rc_data_t *data);


#endif /* __RC_H__ */

