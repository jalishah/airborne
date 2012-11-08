
#ifndef __CHANNEL_MAP_H__
#define __CHANNEL_MAP_H__


#include <stdint.h>


#include "platform.h"


typedef struct
{
   uint8_t *map;
   uint8_t *use_bias;
}
channel_map_t;



void channel_map_init(channel_map_t *channel_map, uint8_t map[MAX_CHANNELS], uint8_t use_bias[MAX_CHANNELS]);


int channel_uses_bias(channel_map_t *channel_map, channel_t channel);


int channel_lookup(channel_map_t *channel_map, channel_t channel);


#endif /* __CHANNEL_MAP_H__ */

