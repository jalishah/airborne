

#ifndef __MOTOSTATE_H__
#define __MOTOSTATE_H__


/* initializes motors state tracker */
void motostate_init(float _ground_max, float _gas_start, float _gas_stop);

/* returns 1 if motor setpoints should be written, otherwise 0 */
int motostate_enabled(void);

/* returns 1 if control system should be active, otherwise 0 */
int motostate_controllable(void);

/* updates the motors state tracker */
void motostate_update(float ground_z, float gas, float dt, int start_allowed);


#endif /* __MOTOSTATE_H__ */

