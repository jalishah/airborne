/*
 * simple_thread.h
 *
 * fixed-priority threads
 *
 *
 *  Created on: 15.06.2010
 *      Author: tobi
 */


#include <stdio.h>
#include <limits.h>

#include "util.h"
#include "../time/ltime.h"
#include "simple_thread.h"
#include "thread_config.h"


void simple_thread_start(simple_thread_t *thread, void *(*func)(void *),
                         char *name, int priority, void *private)
{
   ASSERT_NOT_NULL(thread);
   ASSERT_NOT_NULL(func);
   ASSERT_NOT_NULL(name);
   ASSERT_FALSE(thread->running);

   thread->running = 1;
   thread->name = name;
   thread->private = private;
   (void)pthread_attr_init(&thread->attr);
   (void)pthread_attr_setschedpolicy(&thread->attr, SCHED_FIFO);
   thread->sched_param.sched_priority = priority;
   (void)pthread_attr_setschedparam(&thread->attr, &thread->sched_param);
   (void)pthread_attr_setstacksize(&thread->attr, PTHREAD_STACK_MIN + THREAD_STACK_SIZE);
   (void)pthread_create(&thread->handle, &thread->attr, func, thread);
}


void simple_thread_stop(simple_thread_t *thread)
{
   ASSERT_NOT_NULL(thread);
   ASSERT_TRUE(thread->running);

   thread->running = 0;
   (void)pthread_join(thread->handle, NULL);
}
