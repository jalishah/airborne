
#include <stdarg.h>
#include <stdio.h>
#include <malloc.h>
#include <math.h>
#include <unistd.h>

#include <simple_thread.h>
#include <core.pb-c.h>
#include <util.h>
#include <sclhelper.h>

#include "cmd.h"
#include "../util/logger/logger.h"
#include "../controllers/ctrl.h"
#include "../model/model.h"
#include "../sensor_actor/interfaces/gpio.h"
#include "../sensor_actor/holger_fc/holger_fc.h"
#include "../platform/platform.h"


#define THREAD_NAME     "cmd_interface"
#define THREAD_PRIORITY 1


static void *cmd_socket = NULL;
static simple_thread_t thread;
static float *rpm = NULL;


static void spin_down(void)
{
   /* switch off motors command: */
   fc_stop_motors();
   
   /* wait until all motors are stopped: */
   while (1)
   {
      msleep(10);
      fc_read_motors_rpm(rpm);
      int quit = 1;
      for (int i = 0; i < platform_motors(); i++)
      {
         if (rpm[i] > 10.0)
         {
            quit = 0;
            break;
         }
      }

      if (quit)
      {
         break;
      }
   }
}


static void spin_up(Reply *reply)
{
   return;
   int retry_count = 0;
retry:
   LOG(LL_DEBUG, "starting motors");
   ctrl_override(0.0f, 0.0f, 0.0f, 0.0f);
   fc_start_motors();
   sleep(1);
   for (int i = 0; i < 6; i++)
   {
      ctrl_override(0.0f, 0.0f, 0.0f, (i & 1)? 0.3f : 0.1f);
      msleep(100);
   }
   int valid_count = 0;
   int fail_timer = 1000;
   while (1)
   {
      msleep(10);
      valid_count++;
      
      if (fail_timer-- == 0)
      {
          LOG(LL_DEBUG, "starting failed, restarting");
          spin_down();
          if (++retry_count == 3)
          {
             reply->status = STATUS__E_HW;
             return;
          }
          goto retry;
      }
      
      /* reset valid_count, if motor rpm too low */
      fc_read_motors_rpm(rpm);
      for (int i = 0; i < platform_motors(); i++)
      {
         if (rpm[i] < 1000.0f || rpm[i] > 50000.0f)
         {
            valid_count = 0;
            break;
         }
      }
      
      if (valid_count > 200)
      {
         ctrl_stop_override();
         return;
      }
   }
}


static void set_ctrl_param(Reply *reply, Request *request)
{
   if (request->ctrl_data == NULL)
   {
      reply->status = STATUS__E_SEM;
   }
   else if (ctrl_set_data(request->ctrl_data->param, request->ctrl_data->val) != 0)
   {
      reply->status = STATUS__E_SEM;
   }
   else
   {
      reply->status = STATUS__OK;
   }
}


static void get_state(Params *params)
{
   params->start_lon = gps_start_lon();
   params->start_lat = gps_start_lat();
   params->start_alt = gps_start_alt();
}


SIMPLE_THREAD_BEGIN(thread_func)
{
   SIMPLE_THREAD_LOOP_BEGIN
   {
      Reply reply = REPLY__INIT;
      reply.status = STATUS__OK;
      unsigned char raw_data[1024];
      int raw_data_size = scl_recv_static(cmd_socket, raw_data, sizeof(raw_data));
      if (raw_data_size < 0)
      {
         LOG(LL_ERROR, "scl recv failed");
         sleep(1);
         continue;
      }
      Request *request = request__unpack(NULL, raw_data_size, raw_data);
      Params params = PARAMS__INIT;
      if (request == NULL)
      {
         reply.status = STATUS__E_SYN;
         LOG(LL_ERROR, "could not parse protobuf message");
      }
      else
      {
         switch (request->type)
         {
            case REQUEST_TYPE__SPIN_UP:
               LOG(LL_DEBUG, "SPIN_UP");
               spin_up(&reply);
               break;

            case REQUEST_TYPE__SPIN_DOWN:
               LOG(LL_DEBUG, "SPIN_DOWN");
               spin_down();
               break;

            case REQUEST_TYPE__RESET_CTRL:
               LOG(LL_DEBUG, "RESET_CTRL");
               ctrl_reset();
               break;

            case REQUEST_TYPE__SET_CTRL_PARAM:
               LOG(LL_DEBUG, "SET_CTRL_PARAM");
               set_ctrl_param(&reply, request);
               break;

            case REQUEST_TYPE__GET_PARAMS:
               LOG(LL_DEBUG, "GET_PARAMS");
               reply.params = &params;
               get_state(&params);
               break;

            default:
               LOG(LL_DEBUG, "unknown request type");
         }
         request__free_unpacked(request, NULL);
      }
      SCL_PACK_AND_SEND_DYNAMIC(cmd_socket, reply, reply);
   }
   SIMPLE_THREAD_LOOP_END
}
SIMPLE_THREAD_END


int cmd_init(void)
{
   ASSERT_ONCE();
   cmd_socket = scl_get_socket("ctrl");
   if (cmd_socket == NULL)
   {
      return -1;
   }
   rpm = malloc(sizeof(float) * platform_motors());
   simple_thread_start(&thread, thread_func, THREAD_NAME, THREAD_PRIORITY, NULL);
   return 0;
}

