/*
 * model.h
 *
 *  Created on: 26.09.2010
 *      Author: tobi
 */

#ifndef __MODEL_H__
#define __MODEL_H__


#include "../geometry/orientation.h"


/*
 * position-speed-acceleration state:
 */
typedef struct
{
   float pos; /* position, in m */
   float speed; /* in m / s */
}
position_state_t;


/*
 * model state:
 */
typedef struct
{
   /* local position estimates: */
   position_state_t x; /* x state */
   position_state_t y; /* y state */
   position_state_t ultra_z; /* ultrasonoc altitude over ground */
   position_state_t baro_z; /* barometric altitude above sea level */
   euler_t euler;
}
model_state_t;


/*
 * model input:
 */
typedef struct
{
   float dt;

   /* positions input: */
   float ultra_z;
   float baro_z;
   float dx;
   float dy;

   /* control acc input: */
   vec3_t acc;
}
model_input_t;


/*
 * state access primitives
 */
void model_init(void);


/*
 * model state update function
 */
void model_step(model_state_t *out, model_input_t *input);


#endif /* __MODEL_H__ */

