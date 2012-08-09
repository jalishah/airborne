
/*
 * generic platform interface
 */


#ifndef __PLATFORM_H__
#define __PLATFORM_H__


#include "../hardware/interfaces/gps.h"
#include "../hardware/interfaces/ahrs.h"
#include "../hardware/interfaces/rc.h"



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
   int (*init)(void);
   float (*read)(void);
   unsigned int cells;
}
batt_t;


typedef struct
{
   int (*init)(void);
   void (*write)(float *forces);
   void (*read)(float *rpm);
   unsigned int count;
}
motors_t;


typedef struct
{
   /* sensors: */
   gps_interface_t *gps;
   ultra_t *ultra;
   baro_t *baro;
   ahrs_t *ahrs;
   rc_interface_t *rc;
   batt_t *batt;
   /* actuators: */
   motors_t *motors;
}
platform_t;



void platforms_init(unsigned int select);

platform_t *platform_create(void);


/*
 * returns the number of motors
 */
int platform_motors(void);


/*
 * start the motors
 */
void platform_start_motors(void);


/*
 * reads the current motor RPM
 */
void platform_read_motors(float *rpm);


/*
 * stops the motors
 */
void platform_stop_motors(void);


void platform_ahrs_read(ahrs_data_t *data);

void platform_gps_read(gps_data_t *data);

float platform_ultra_read(void);

float platform_baro_read(void);


#endif /* __PLATFORM_H__ */

