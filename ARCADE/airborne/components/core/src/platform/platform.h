
/*
 * platform.h
 *
 * platform definitions
 *
 * author: tobi
 */


#ifndef __PLATFORM_H__
#define __PLATFORM_H__


#include "mixer.h"


void platform_init(void);

int platform_motors(void);

mixer_t *platform_mixer(void);


#endif /* __PLATFORM_H__ */

