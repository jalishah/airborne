
/*
 * ahrs.h
 *
 * Created on: 11.06.2010
 * Author: tobi
 */


#ifndef __AHRS_H__
#define __AHRS_H__


typedef struct
{
   /* euler angles: */
   float pitch; /* -PI .. PI, 0 is horizontal */
   float roll; /* -PI .. PI, 0 is horizontal */
   float yaw; /* -PI .. PI, 0 is north */

   /* angular speeds: */
   float pitch_rate; /* in rad / s */
   float roll_rate; /* in rad / s */
   float yaw_rate;  /* in rad / s */

   /* acc acceleration: */
   float acc_pitch; /* in m / (s ^ 2) */
   float acc_roll; /* in m / (s ^ 2) */
   float acc_yaw;  /* in rad / (s ^ 2) */
}
ahrs_data_t;


#define AHRS_DATA_INITIALIZER  {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}


int ahrs_init(void);

int ahrs_read(ahrs_data_t *data);


#endif /* __AHRS_H__ */

