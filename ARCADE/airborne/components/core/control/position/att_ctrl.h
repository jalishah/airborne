
#ifndef __ATT_CTRL_H__
#define __ATT_CTRL_H__


void att_ctrl_init(void)

void att_ctrl_reset(void)

void att_ctrl_step(float out[2], const float dt, const float pos[2], const float setp[2])
 

#endif /* __ATT_CTRL_H__ */

