
#ifndef __RC_CHANNELS_H__
#define __RC_CHANNELS_H__


#include <stdint.h>

#include "deadzone.h"


#define MAX_CHANNELS 5


typedef enum
{
   CH_PITCH,
   CH_ROLL,
   CH_YAW,
   CH_GAS,
   CH_SWITCH
}
channel_t;


typedef struct
{
   uint8_t *map;
   float *scale;
   deadzone_t *deadzone;
}
rc_channels_t;


void rc_channels_init(rc_channels_t *channels, uint8_t map[MAX_CHANNELS], float scale[MAX_CHANNELS], deadzone_t *deadzone);

float rc_channels_get(rc_channels_t *channels, float *raw_channels, channel_t channel);


#endif /* __RC_CHANNELS__ */

