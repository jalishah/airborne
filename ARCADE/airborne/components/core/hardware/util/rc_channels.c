
#include <util.h>

#include "rc_channels.h"


static int ch_is_symmetric[MAX_CHANNELS] = {1, 1, 1, 0, 0};


void rc_channels_init(rc_channels_t *channels, uint8_t map[MAX_CHANNELS], float scale[MAX_CHANNELS], deadzone_t *deadzone)
{
   channels->map = map;
   channels->scale = scale;
   channels->deadzone = deadzone;
}


float rc_channels_get(rc_channels_t *channels, float *raw_channels, channel_t channel)
{
   /* perform channel mapping: */
   int raw_index = channels->map[channel];

   /* read raw channel: */
   float raw = raw_channels[raw_index];
   
   /* scale or apply deadzone, depends on symmetry flag: */
   if (!ch_is_symmetric[channel])
   {
      raw = (1.0f + raw) / 2.0f;
   }
   else if (channels->deadzone)
   {
      raw = deadzone_calc(channels->deadzone, raw);
   }

   /* scale and constrain output: */
   raw *= channels->scale[channel];
   if (ch_is_symmetric[channel])
   {
      raw =  sym_limit(raw, 1.0f);
   }
   else
   {
      raw = limit(raw, 0.0f, 1.0f);
   }
   return raw;
}

