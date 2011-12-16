/*
 * remote_control.h
 *
 *  Created on: 11.06.2010
 *      Author: tobi
 */


#ifndef __REMOTE_CONTROL_H__
#define __REMOTE_CONTROL_H__


typedef struct
{
   short channels[11];
}
rc_data_t;


#define RC_DATA_INITIALIZER {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}


#endif /* __REMOTE_CONTROL_H__ */

