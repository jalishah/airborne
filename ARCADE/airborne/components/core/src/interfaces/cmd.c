
#include <stdarg.h>
#include <stdio.h>
#include <malloc.h>
#include <math.h>
#include <unistd.h>

#include "cmd.h"
#include "core.pb-c.h"
#include "../util/util.h"
#include "../util/logger/logger.h"
#include "../controllers/ctrl.h"
#include "../model/model.h"
#include "../util/threads/simple_thread.h"
#include "../sensor_actor/interfaces/gpio.h"
#include "../sensor_actor/mk-fc/mk-fc.h"
#include "../platform/platform.h"
#include "../../../../../common/scl/src/sclhelper.h"


#define THREAD_NAME     "cmd_interface"
#define THREAD_PRIORITY 1


static void *socket = NULL;
static simple_thread_t thread;
static float *rpm = NULL;


static void halt_motors(void)
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


static void spin_up_motors(Reply *reply)
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
          halt_motors();
          if (++retry_count == 3)
          {
             reply->status = STATUS__E_HW;
             reply->message = "could not spin up motors";
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


static int value_len_ok(Request *request)
{
   size_t valid_len;
   if (request->ctrl->type == CTRL_TYPE__GPS)
   {
      valid_len = 2;
   }
   else
   {
      valid_len = 1;   
   }
   return request->ctrl->n_pos == valid_len;
}


static void set_setpoint(Reply *reply, Request *request)
{
   if (request->ctrl == NULL)
   {
      reply->status = STATUS__E_ARG;
      reply->message = "ctrl field missing";
   }
   else
   {
      if (request->ctrl->pos == NULL)
      {
         reply->status = STATUS__E_ARG;
         reply->message = "ctrl pos field missing";
      }
      else
      {
         if (!value_len_ok(request))
         {
            reply->status = STATUS__E_ARG;
            reply->message = "ctrl pos field has invalid length";
         }
         else
         {
            float *speed = request->ctrl->has_speed ? &request->ctrl->speed : NULL;
            LOG(LL_DEBUG, "speed: %p", speed);
            int status = ctrl_set_setpoint(request->ctrl->type, request->ctrl->pos, speed);
            if (status != 0)
            {
               reply->status = STATUS__E_ARG;
               reply->message = "could not set setpoint";
            }
         }
      }
   }
}


static size_t get_state(float *val, StateType type)
{
   size_t size = 1;
   switch (type)
   {
      case STATE_TYPE__ULTRA_ALT:
         *val = model_get_ultra_alt();
         break;

      case STATE_TYPE__BARO_ALT:
         *val = model_get_baro_alt();
         break;

      case STATE_TYPE__GPS_START:
         val[0] = gps_start_lon();
         val[1] = gps_start_lat();
         size = 2;
         break;

      case STATE_TYPE__GPS_REL:
         val[0] = model_get_x();
         val[1] = model_get_y();
         size = 2;
         break;

      case STATE_TYPE__YAW_POS:
         *val = model_get_yaw();
         break;

      case STATE_TYPE__VOLTAGE:
         *val = 0.0f;
         break;

      case STATE_TYPE__RC_QUAL:
         *val = 0.0f;
         break;
   }
   return size;
}


SIMPLE_THREAD_BEGIN(thread_func)
{
   SIMPLE_THREAD_LOOP_BEGIN
   {
      Reply reply = REPLY__INIT;
      reply.status = STATUS__OK;
      reply.message = NULL;
      unsigned char raw_data[1024];
      int raw_data_size = scl_recv_static(socket, raw_data, sizeof(raw_data));
      if (raw_data_size < 0)
      {
         LOG(LL_ERROR, "scl recv failed");
         continue;
      }
      float val_buf[2];
      Request *request = request__unpack(NULL, raw_data_size, raw_data);
      if (request == NULL)
      {
         reply.status = STATUS__E_PB;
         reply.message = "could not parse input";
         LOG(LL_ERROR, reply.message);
      }
      else
      {
         switch (request->type)
         {
            case REQUEST_TYPE__POWER_ON:
               gpio_set(GPIO_POWER, 1);
               break;

            case REQUEST_TYPE__POWER_OFF:
               gpio_set(GPIO_POWER, 0);
               break;

            case REQUEST_TYPE__START_MOTORS:
               spin_up_motors(&reply);
               break;

            case REQUEST_TYPE__STOP_MOTORS:
               halt_motors();
               break;

            case REQUEST_TYPE__CTRL_SET_SP:
               set_setpoint(&reply, request);
               break;

            case REQUEST_TYPE__CTRL_GET_ERR:
               if (request->ctrl == NULL)
               {
                  reply.status = STATUS__E_ARG;
                  reply.message = "ctrl field missing";
               }
               else
               {
                  reply.value = val_buf;
                  reply.n_value = ctrl_get_err(reply.value, request->ctrl->type);
               }
               break;

            case REQUEST_TYPE__CTRL_RESET:
               ctrl_reset();
               break;

            case REQUEST_TYPE__STATE_GET:
               if (!request->has_state_type)
               {
                  reply.status = STATUS__E_ARG;
                  reply.message = "state type field missing";
               }
               else
               {
                  reply.value = val_buf;
                  reply.n_value = get_state(reply.value, request->state_type);
               }
               break;
         }
         request__free_unpacked(request, NULL);
      }

      unsigned int reply_data_len = reply__get_packed_size(&reply);
      void *buffer = malloc(reply_data_len);
      ASSERT_NOT_NULL(buffer);
      reply__pack(&reply, buffer);
      scl_send_dynamic(socket, buffer, reply_data_len, 0);
   }
   SIMPLE_THREAD_LOOP_END
}
SIMPLE_THREAD_END


int cmd_init(void)
{
   ASSERT_NULL(socket);
   socket = scl_get_socket("ctrl");
   if (socket == NULL)
   {
      return -1;
   }
   rpm = malloc(sizeof(float) * platform_motors());
   simple_thread_start(&thread, thread_func, THREAD_NAME, THREAD_PRIORITY, NULL);
   return 0;
}

