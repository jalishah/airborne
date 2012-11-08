
#ifndef __RC_DSL_DRIVER_H__
#define __RC_DSL_DRIVER_H__


#include "../../libs/rc_dsl/rc_dsl.h"


int rc_dsl_driver_init(void);

int rc_dsl_driver_read(float channels_out[RC_DSL_CHANNELS]);


#endif /* __RC_DSL_DRIVER_H__ */

