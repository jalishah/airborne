

/*
 * generic platform interface
 */


#include "platform.h"
#include "quadro.h"

#include <string.h>
#include <malloc.h>
#include <assert.h>


static platform_t **platforms = NULL;
static platform_t *platform = NULL;

#define N_PLATFORMS 1


void platforms_init(unsigned int select)
{
   assert(select < N_PLATFORMS);
   platforms = malloc(N_PLATFORMS * sizeof(platform_t *));
   platforms[0] = quadro_create();
   platform = platforms[select];
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

