/*
 * simple_thread.h
 *
 * fixed-priority threads
 *
 *
 *  Created on: 15.06.2010
 *      Author: tobi
 */


#ifndef SIMPLE_THREAD_H
#define SIMPLE_THREAD_H


#include <pthread.h>

#include "thread_realtime.h"


typedef struct
{
   char *name;
   pthread_t handle;
   volatile int running;
   pthread_attr_t attr;
   struct sched_param sched_param;
   void *private;
}
simple_thread_t;


#define SIMPLE_THREAD_BEGIN(name) \
   static void *name(void *__arg) \
   { \
      simple_thread_t *thread = (simple_thread_t *)__arg;

#define SIMPLE_THREAD_LOOP_BEGIN \
   thread_stack_prefault(); \
   while (thread->running) \
   {

#define SIMPLE_THREAD_LOOP_END \
   }

#define SIMPLE_THREAD_END \
   return NULL; }


void simple_thread_start(simple_thread_t *thread, void *(*func)(void *),
                         char *name, int priority, void *private);

void simple_thread_stop(simple_thread_t *thread);


#endif /* SIMPLE_THREAD_H */
