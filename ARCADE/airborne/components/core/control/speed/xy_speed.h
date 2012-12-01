

#ifndef __XY_SPEED_H__
#define __XY_SPEED_H__


#include "../../util/math/vec2.h"


void xy_speed_ctrl_init(void);


void xy_speed_ctrl_run(vec2_t *control, vec2_t *speed_setpoint, vec2_t *speed, float yaw);


#endif /* __XY_SPEED_H__ */

