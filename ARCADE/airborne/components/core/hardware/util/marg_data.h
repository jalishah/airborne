
#ifndef __MARG_DATA_H__
#define __MARG_DATA_H__


#include "../../geometry/orientation.h"


typedef struct
{
   vec3_t gyro;
   vec3_t acc;
   vec3_t mag;
}
marg_data_t;


#endif /* __MARG_DATA_H__ */

