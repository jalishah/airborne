/*
 * health.h
 *
 *  Created on: 11.06.2010
 *      Author: tobi
 */


#ifndef COCO_HEALTH_H
#define COCO_HEALTH_H


#include <sys/time.h>
#include <pthread.h>


typedef struct
{
   float voltage;
   float current;
   float signal;
   float servo_nick; /* TODO: i don't belong here! */
   pthread_mutex_t mutex;
}
health_data_t;


#define HEALTH_DATA_INITIALIZER {0, 0, 0, 0, PTHREAD_MUTEX_INITIALIZER}


typedef struct
{
   int (*read)(health_data_t *data);
   int (*initialize)(void);
   int (*finalize)(void);
   void (*wait_for_event)(void);
}
health_device_t;


#endif /* COCO_HEALTH_H */

