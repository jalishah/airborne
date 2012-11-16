//=====================================================================================================
// MadgwickAHRS.h
//=====================================================================================================
//
// Implementation of Madgwick's IMU and AHRS algorithms.
// See: http://www.x-io.co.uk/node/8#open_source_ahrs_and_imu_algorithms
//
// Date			Author          Notes
// 29/09/2011	SOH Madgwick    Initial release
// 02/10/2011	SOH Madgwick	Optimised for reduced CPU load
//
//=====================================================================================================

#ifndef __AHRS_H__
#define __AHRS_H__


#include "../hardware/util/marg_data.h"


typedef struct
{
   float beta; /* 2 * beta (Kp) */
   float beta_step;
   float beta_end;
   quat_t quat; /* quaternion of sensor frame relative to auxiliary frame */
}
ahrs_t;


void ahrs_init(ahrs_t *ahrs, float beta_start, float beta_step, float beta_end);

/*
 * returns -1 if the ahrs is not ready
 *          1 if the ahrs became ready
 *          0 on normal operation
 */
int ahrs_update(ahrs_t *ahrs, marg_data_t *marg_data, float accelCutoff, float dt);


#endif /* __AHRS_H__ */
