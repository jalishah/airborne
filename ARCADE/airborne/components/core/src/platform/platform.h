
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


/*
 * initializes the platform
 */
void platform_init(void);


/*
 * returns number of motors on platform
 */
int platform_motors(void);


/*
 * returns a mixer matrix fpr the platform
 */
mixer_t *platform_mixer(void);


#endif /* __PLATFORM_H__ */

