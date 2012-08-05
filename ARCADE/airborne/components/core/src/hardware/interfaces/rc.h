
/*
 * rc.h
 *
 * author: Tobias Simon, Ilmenau University of Technology
 */


#ifndef __RC_H__
#define __RC_H__


#define EXTRA_RC_CHANNELS (2)


typedef struct
{
   short pitch;
   short roll;
   short yaw;
   short gas;
   short extra[EXTRA_RC_CHANNELS];
}
rc_data_t;



#endif /* __RC_H__ */

