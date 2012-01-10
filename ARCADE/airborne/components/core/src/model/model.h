/*
 * model.h
 *
 *  Created on: 26.09.2010
 *      Author: tobi
 */

#ifndef __MODEL_H__
#define __MODEL_H__


#include "../sensor_actor/interfaces/altimeter.h"
#include "../sensor_actor/interfaces/ahrs.h"
#include "../sensor_actor/interfaces/gps.h"


/*
 * position-speed-acceleration state:
 */
typedef struct
{
   float pos; /* position, in m */
   float speed; /* in m / s */
   float acc; /* acceleration, in m / s ^ 2 */
}
position_state_t;


/*
 * angle-speed-acceleration state:
 */
typedef struct
{
   float angle; /* angle, in rad */
   float speed; /* in rad / s */
}
angle_state_t;



/*
 * model state:
 */
typedef struct
{
   position_state_t x; /* x state */
   position_state_t y; /* y state */
   position_state_t ultra_z; /* ultrasonoc altitude over ground */
   position_state_t baro_z; /* barometric altitude above sea level */
   
   angle_state_t yaw;
   angle_state_t pitch;
   angle_state_t roll;
   
   float gps_ground_speed;
   float gps_yaw;
}
model_state_t;


/*
 * model input:
 */
typedef struct
{
   float ultra_z;
   float baro_z;
   ahrs_data_t ahrs_data;
   gps_data_t gps_data;
   float dt;
}
model_input_t;


/*
 * state access primitives
 */
void model_init(void);

float model_get_yaw(void);

float model_get_ultra_alt(void);

float model_get_baro_alt(void);

float model_get_x(void);

float model_get_y(void);


/*
 * model state update function
 */
void model_step(model_state_t *out, model_input_t *input);


#endif /* __MODEL_H__ */

