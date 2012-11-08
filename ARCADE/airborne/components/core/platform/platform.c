

/*
 * generic platform interface
 */


#include "platform.h"
#include "quadro.h"

#include <string.h>
#include <malloc.h>
#include <assert.h>
#include <errno.h>


#define N_PLATFORMS 1
static platform_t **_platforms = NULL;

static platform_t *platform = NULL;



void platforms_init(unsigned int select)
{
   assert(select < N_PLATFORMS);
   _platforms = malloc(N_PLATFORMS * sizeof(platform_t *));
   _platforms[0] = quadro_create();
   platform = _platforms[select];
}



#define CHECK_DEV(component)\
   if (!component) \
   { \
      return -ENODEV; \
   }


platform_t *platform_create(void)
{
   platform_t *plat = malloc(sizeof(platform_t));
   memset(plat, 0, sizeof(platform_t));
   return plat;
}


int platform_write_motors(float forces[4], float voltage)
{
   CHECK_DEV(platform->motors);
   return platform->motors->write(forces, voltage);
}


int platform_read_rc(float channels[MAX_CHANNELS])
{
   CHECK_DEV(platform->rc);
   platform->rc->read(channels);
   return 0;
}


int platform_read_ahrs(ahrs_data_t *data)
{
   
}


int platform_read_gps(gps_data_t *data)
{
   
}


int platform_read_ultra(float *data)
{
   return 0;   
}


int platform_read_baro(float *data)
{
   return 0;   
}


int platform_read_voltage(float *voltage)
{
   CHECK_DEV(platform->voltage);
   *voltage = platform->voltage->read();
   return 0;
}


