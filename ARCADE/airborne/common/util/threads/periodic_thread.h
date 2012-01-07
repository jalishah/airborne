/*
 * periodic_thread.h
 *
 * periodic, fixed-priority threads
 *
 *
 *  Created on: 15.06.2010
 *      Author: tobi
 */


#ifndef PERIODIC_THREAD_H
#define PERIODIC_THREAD_H


#include <time.h>
#include <pthread.h>


#include "periodic_thread.h"
#include "thread_realtime.h"


typedef struct
{
   struct timespec now;
   struct timespec next;
   struct timespec period;
}
period_t;


typedef struct
{
   const char *name;
   period_t periodic_data;
   pthread_t handle;
   volatile int running;
   pthread_attr_t attr;
   struct sched_param sched_param;
   void *private;
}
periodic_thread_t;


#define PERIODIC_THREAD_BEGIN(name) \
   static void *name(void *__arg) \
   { \
      periodic_thread_t *thread = (periodic_thread_t *)__arg; \
       
#define PERIODIC_THREAD_LOOP_BEGIN \
   thread_stack_prefault(); \
   periodic_thread_init_period(thread); \
   while (thread->running) \
   { \
      periodic_thread_wait_for_next_period(thread);

#define PERIODIC_THREAD_LOOP_END \
   }

#define PERIODIC_THREAD_END \
   return NULL; }


void periodic_thread_start(periodic_thread_t *thread, void *(*func)(void *),
                           const char *name, int priority, struct timespec period, void *private);

void periodic_thread_stop(periodic_thread_t *thread);


void periodic_thread_init_period(periodic_thread_t *thread);


int periodic_thread_wait_for_next_period(periodic_thread_t *thread);


#endif /* PERIODIC_THREAD_H */
