
/*
 * generic platform interface
 */


#ifndef __PLATFORM_H__
#define __PLATFORM_H__


typedef struct
{
   void (*read)(gps_data_t *data);
   float covar;
}
gps_t;


typedef struct
{
   float (*read)(void);
   float covar;
}
ultra_t;


typedef struct
{
   float (*read)(void);
   float covar;
}
baro_t;


typedef struct
{
   void (*read)(ahrs_data_t *data);
   float acc_covar;
}
ahrs_t;


typedef struct
{
   void (*read)(rc_data_t *data);
}
rc_t;


typedef struct
{
   float (*read)(void);
   unsigned int cells;
}
batt_t;


typedef struct
{
   void (*write)(float *forces);
   void (*read)(float *rpm);
   unsigned int n;
}
motors_t;


typedef struct
{
   /* sensors: */
   gps_t *gps;
   ultra_t *ultra;
   baro_t *baro;
   ahrs_t *ahrs;
   rc_t *rc;
   batt_t *batt;
   /* actuators: */
   motors_t *motors;
}
platform_t;



extern platform_t platform;



#endif /* __PLATFORM_H__ */

