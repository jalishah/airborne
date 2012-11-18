
#ifndef __ATT_CTRL_H__
#define __ATT_CTRL_H__


#include "../../util/math/vec2.h"


void att_ctrl_init(void);

void att_ctrl_reset(void);

void att_ctrl_step(vec2_t *ctrl, const float dt, const vec2_t *pos, const vec2_t *setp);
 

#endif /* __ATT_CTRL_H__ */

