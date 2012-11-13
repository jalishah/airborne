

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
   int raw_index = channels->map[channel];
   float raw = raw_channels[raw_index];
   if (!ch_is_symmetric[channel])
   {
      /* shift and scale non-symmetric stick: */
      raw = (1.0 + raw) / 2.0;
   }
   else if (channels->deadzone)
   {
      /* apply deadzone if provided: */
      raw = deadzone_calc(channels->deadzone, raw);
   }
   return raw * channels->scale[channel];
}

