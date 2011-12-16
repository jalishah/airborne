
/*
 * platform.c
 *
 * platform definitions
 *
 * author: tobi
 */



#include "platform.h"
#include "hexa_platform.h"
#include "quadro_platform.h"


void platform_init(void)
{
   hexa_platform_init();
}


int platform_motors(void)
{
   return hexa_platform_motors();
}


mixer_t *platform_mixer(void)
{
   return hexa_platform_mixer();  
}

