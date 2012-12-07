
#ifndef __FLIGHT_DETECT_H__
#define __FLIGHT_DETECT_H__


#include <stddef.h>


typedef enum
{
   FS_STANDING,
   FS_FLYING,
   FS_CRASHED
}
flight_state_t;


void flight_detect_init(size_t window, size_t hysteresis, float fly_tresh, float crash_tresh);


flight_state_t flight_detect(float acc[3]);


#endif /* __FLIGHT_DETECT_H__ */

