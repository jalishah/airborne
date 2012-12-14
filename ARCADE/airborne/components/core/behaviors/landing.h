

#ifndef __LANDING_H__
#define __LANDING_H__


void landing_init(void);

int landing_started(void);

int landing_run(float *z_ctrl_gas, float ultra_alt, float baro_alt, float dt);


#endif /* __LANDING_H__ */

