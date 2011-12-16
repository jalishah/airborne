/*
 * thread_realtime.c
 *
 *  Created on: 17.06.2010
 *      Author: tobi
 */


#include <string.h>

#include "thread_config.h"


void thread_stack_prefault(void)
{
   char dummy[THREAD_STACK_SIZE];
   memset(&dummy, 0, THREAD_STACK_SIZE);
}
