
/*
 * quadro_platform.h
 */


#ifndef __QUADRO_PLATFORM_H__
#define __QUADRO_PLATFORM_H__


#include "platform.h"


void quadro_platform_init(void);

int quadro_platform_motors(void);

mixer_t *quadro_platform_mixer(void);


#endif /* __QUADRO_PLATFORM_H__ */

