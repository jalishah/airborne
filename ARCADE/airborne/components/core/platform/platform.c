

/*
 * generic platform interface
 */


#include "platform.h"
#include "quadro.h"

#include <malloc.h>
#include <assert.h>


platform_t *platform = NULL;
static platform_t **platforms = NULL;

#define N_PLATFORMS 1


void platforms_init(unsigned int select)
{
   assert(select < N_PLATFORMS);
   platforms = malloc(N_PLATFORMS * sizeof(platform_t *));
   platforms[0] = quadro_platform_create();
   platform = platforms[select];
}


platform_t *platform_create(void)
{
   platform_t *plat = malloc(sizeof(platform_t));
   memset(plat, 0, sizeof(platform_t));
   return plat;
}

