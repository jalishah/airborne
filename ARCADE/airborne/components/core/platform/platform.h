
/*
 * generic platform interface
 */


#ifndef __PLATFORM_H__
#define __PLATFORM_H__


#include "../hardware/interfaces/gps.h"
#include "../hardware/interfaces/ahrs.h"
#include "../hardware/interfaces/rc.h"
#include "../hardware/interfaces/motors.h"
#include "../hardware/interfaces/voltage.h"


typedef struct
{
   int (*init)(void);
   float (*read)(void);
   float covar;
}
ultra_t;


typedef struct
{
   int (*init)(void);
   float (*read)(void);
   float covar;
}
baro_t;


typedef struct
{
   int (*init)(void);
   void (*read)(ahrs_data_t *data);
   float acc_covar;
}
ahrs_t;


typedef struct
{
   /* sensors: */
   gps_interface_t *gps;
   ultra_t *ultra;
   baro_t *baro;
   ahrs_t *ahrs;
   rc_interface_t *rc;
   voltage_interface_t *voltage;
   /* actuators: */
   motors_interface_t *motors;
}
platform_t;



void platforms_init(unsigned int select);

platform_t *platform_create(void);

int platform_write_motors(float forces[4], float voltage);

int platform_read_rc(float channels[MAX_CHANNELS]);

int platform_read_ahrs(ahrs_data_t *data);

int platform_read_gps(gps_data_t *data);

int platform_read_ultra(float *data);

int platform_read_baro(float *data);

int platform_read_voltage(float *voltage);


#endif /* __PLATFORM_H__ */

