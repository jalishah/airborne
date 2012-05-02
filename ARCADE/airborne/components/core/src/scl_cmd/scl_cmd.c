
#include <stdarg.h>
#include <stdio.h>
#include <malloc.h>
#include <math.h>
#include <unistd.h>

#include <simple_thread.h>
#include <core.pb-c.h>
#include <util.h>
#include <sclhelper.h>

#include "scl_cmd.h"
#include "../util/logger/logger.h"
#include "../controllers/ctrl.h"
#include "../model/model.h"
#include "../sensor_actor/interfaces/gpio.h"
#include "../sensor_actor/holger_fc/holger_fc.h"
#include "../platform/platform.h"
#include "../controllers/z_ctrl.h"
#include "../controllers/navi.h"
#include "../controllers/yaw_ctrl.h"


#define THREAD_NAME     "cmd_interface"
#define THREAD_PRIORITY 1


static void *cmd_socket = NULL;
static simple_thread_t thread;
static float *rpm = NULL;


static void spin_down(void)
{
   return;
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
         printf("rpm: %f\n", rpm[i]);
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
          LOG(LL_ERROR, "starting failed, restarting");
          spin_down();
          if (++retry_count == 3)
          {
             reply->err_msg = "could not spin up motors";
             LOG(LL_ERROR, reply->err_msg);
             reply->status = STATUS__E_HARDWARE;
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


int set_ctrl_param(CtrlParam param, float value)
{
   int status = 0;
   switch (param)
   {
      case CTRL_PARAM__POS_X:
      {
         LOG(LL_DEBUG, "x pos update: %f", value);
         navi_set_dest_x(value); // can't fail
         break;
      }

      case CTRL_PARAM__POS_Y:
      {
         LOG(LL_DEBUG, "y pos update: %f", value);
         navi_set_dest_y(value); // can't fail
         break;
      }

      case CTRL_PARAM__POS_Z_GROUND:
      {
         LOG(LL_DEBUG, "ground z pos update: %f", value);
         z_setpoint_t setpoint = {value, 1};
         z_ctrl_set_setpoint(setpoint); // can't fail
         break;
      }

      case CTRL_PARAM__POS_Z:
      {
         LOG(LL_DEBUG, "z pos update: %f", value);
         z_setpoint_t setpoint = {value, 0};
         z_ctrl_set_setpoint(setpoint);
         break;
      }

      case CTRL_PARAM__POS_YAW:
      {
         LOG(LL_DEBUG, "yaw pos update: %f", value);
         status = yaw_ctrl_set_pos(value);
         break;
      }
      
      case CTRL_PARAM__SPEED_XY:
      {
         LOG(LL_DEBUG, "xy speed update: %f", value);
         status = navi_set_travel_speed(value);
         break;
      }

      case CTRL_PARAM__SPEED_Z:
      {
         LOG(LL_ERROR, "[not implemented] z speed update: %f", value);
         break;
      }

      case CTRL_PARAM__SPEED_YAW:
      {
         LOG(LL_DEBUG, "yaw speed update: %f", value);
         status = yaw_ctrl_set_speed(value);
         break;
      }
   }
   return status;
}



static void check_and_set_ctrl_param(Reply *reply, Request *request)
{
   if (request->ctrl_data == NULL)
   {
      reply->err_msg = "no ctrl_data provided";
      LOG(LL_ERROR, reply->err_msg);
      reply->status = STATUS__E_SEMANTIC;
   }
   else if (set_ctrl_param(request->ctrl_data->param, request->ctrl_data->val) != 0)
   {
      reply->err_msg = "parameter value out of range";
      LOG(LL_ERROR, reply->err_msg);
      reply->status = STATUS__E_SEMANTIC;
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
   params->setp_x = navi_get_dest_x();
   params->setp_y = navi_get_dest_y();
   float setp_z = z_ctrl_get_setpoint();
   if (z_ctrl_mode_is_ground())
   {
      params->setp_z_ground = setp_z;
   }
   else
   {
      params->setp_z = setp_z;
   }
   params->setp_yaw = yaw_ctrl_get_pos();
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
         reply.status = STATUS__E_SYNTAX;
         reply.err_msg = "could not parse protobuf message";
         LOG(LL_ERROR, reply.err_msg);
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
               check_and_set_ctrl_param(&reply, request);
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

