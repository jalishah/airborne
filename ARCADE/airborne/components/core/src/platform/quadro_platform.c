
/*
 * hexa_platform.c
 */

#include "platform.h"
#include "util.h"

#define N_MOTORS (4)

mixer_t *mixer = NULL;


void quadro_platform_init(void)
{
   ASSERT_ONCE();
   float init[N_MOTORS * 4] = 
   {  /*       gas   pitch  roll   yaw */
      /* m1 */ 1.0f,  1.0f,  0.0f,  1.0f,
      /* m2 */ 1.0f, -1.0f,  0.0f,  1.0f,
      /* m3 */ 1.0f,  0.0f, -1.0f, -1.0f,
      /* m4 */ 1.0f,  0.0f,  1.0f, -1.0f,
   };
   mixer = mixer_create(N_MOTORS, init);
}


int quadro_platform_motors(void)
{
   return N_MOTORS;
}


mixer_t *quadro_platform_mixer(void)
{
   return mixer;
}

