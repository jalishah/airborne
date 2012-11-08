

#include "channel_map.h"


void channel_map_init(channel_map_t *channel_map, uint8_t map[MAX_CHANNELS], uint8_t use_bias[MAX_CHANNELS])
{
   channel_map->map = map;
   channel_map->use_bias = use_bias;
}


int channel_uses_bias(channel_map_t *channel_map, channel_t channel)
{
   return channel_map->use_bias[channel];
}


int channel_lookup(channel_map_t *channel_map, channel_t channel)
{
   return channel_map->map[channel]; 
}

