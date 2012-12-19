
#ifndef __FLIGHT_STATE_H__
#define __FLIGHT_STATE_H__


#include <stddef.h>


typedef enum
{
   FS_STANDING,
   FS_FLYING
}
flight_state_t;


void flight_state_init(size_t window, size_t hysteresis, float fly_tresh, float crash_tresh, float min_ground_z);


flight_state_t flight_state_update(float acc[3], float ground_z);


#endif /* __FLIGHT_STATE_H__ */

