
/*
 * fg_reader.c
 *
 * author: tobi
 */


#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <sys/time.h>
#include <math.h>
#include <string.h>

#include "fg_reader.h"
#include "udp4.h"
#include "../../util/threads/simple_thread.h"
#include "../../util/threads/threadsafe_types.h"



#define THREAD_NAME       "fg_reader"
#define THREAD_PRIORITY   0



static simple_thread_t thread;
static udp_socket_t *reader_socket;
static float float_vals[9];

SIMPLE_THREAD_BEGIN(thread_func)
{
   SIMPLE_THREAD_LOOP_BEGIN
   {
      /* pitch, roll, yaw, acc-x, acc-y, acc-z, lon, lat, ground_alt */
      char buffer[1024];
      udp_socket_recv(reader_socket, buffer, 1024, NULL);
      for (int i = 0; i < sizeof(float_vals) / sizeof(float); i++)
      {
         long x = ntohl(((long *)buffer)[i]);
         memcpy(&float_vals[i], &x, sizeof(float));
      }
      printf("%f %f %f\n", float_vals[0], float_vals[1], float_vals[2]);
   }
   SIMPLE_THREAD_LOOP_END
}
SIMPLE_THREAD_END


void fg_reader_start(void)
{
   reader_socket = udp_socket_create("127.0.0.1", 12346, 255, 1);
   simple_thread_start(&thread, thread_func,
                       THREAD_NAME, THREAD_PRIORITY, NULL);
}

