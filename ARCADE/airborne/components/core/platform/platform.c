

/*
 * generic platform interface
 */


#include "platform.h"
#include "quadro.h"

#include <string.h>
#include <malloc.h>
#include <assert.h>


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


platform_t *platform_create(void)
{
   platform_t *plat = malloc(sizeof(platform_t));
   memset(plat, 0, sizeof(platform_t));
   return plat;
}


int platform_motors(void)
{
   return platform->motors->count;
}


void platform_start_motors(void)
{
   
}


void platform_read_motors(float *rpm)
{

}


void platform_stop_motors(void)
{

}

void platform_ahrs_read(ahrs_data_t *data)
{
   
}

void platform_gps_read(gps_data_t *data)
{
   
}

float platform_ultra_read(void)
{
   return 0.0;   
}

float platform_baro_read(void)
{
   return 0.0;   
}


