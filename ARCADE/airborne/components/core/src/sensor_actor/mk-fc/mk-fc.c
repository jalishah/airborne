/*
 * interface.h
 *
 *  Created on: 15.06.2010
 *      Author: tobi
 */


#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#include "mk-fc.h"
#include "util.h"
#include "data_formats.h"
#include "commands.h"
#include "addresses.h"
#include "frame.h"
#include "../../platform/platform.h"
#include "../../algorithms/average.h"
#include "../../util/logger/logger.h"
#include "../../util/serial/serial.h"
#include "../../util/config/config.h"
#include "../../util/math/lmath.h"
#include "../../util/time/ltime.h"
#include "../../util/threads/threadsafe_types.h"
#include "../../util/threads/simple_thread.h"
#include "../../util/threads/periodic_thread.h"


static int is_initialized = 0;
static serialport_t port;
static int setting;

static const char *serial_port;

static config_t options[] =
{
   {"serial_port", &serial_port},
   {"setting", &setting},
   {NULL, NULL}
};

#define DREQ_THREAD_TIMEOUT_MS 1000
#define DREQ_THREAD_NAME       "debug-requester"
#define DREQ_THREAD_PRIORITY   0
static periodic_thread_t dreq_thread;


#define SREAD_THREAD_NAME      "serial-reader"
#define SREAD_THREAD_PRIORITY  0
static simple_thread_t sread_thread;


static float start_baro = 0.0;

static threadsafe_float_t altitude;
static health_data_t health_data;

static float *rpm = NULL;
static pthread_mutex_t rpm_mutex = PTHREAD_MUTEX_INITIALIZER;


void fc_read_motors_rpm(float *rpm_out)
{
   pthread_mutex_lock(&rpm_mutex);
   memcpy(rpm_out, rpm, sizeof(float) * platform_motors());
   pthread_mutex_unlock(&rpm_mutex);   
}



int health_read(health_data_t *data_out)
{
   ASSERT_NOT_NULL(data_out);
   ASSERT_TRUE(sread_thread.running);

   pthread_mutex_lock(&health_data.mutex);
   *data_out = health_data;
   pthread_mutex_unlock(&health_data.mutex);
   return 0;
}


float fc_read_alt(void)
{
   ASSERT_TRUE(sread_thread.running && dreq_thread.running);
   return threadsafe_float_get(&altitude);
}


pthread_cond_t ack = PTHREAD_COND_INITIALIZER;
pthread_mutex_t ack_mutex = PTHREAD_MUTEX_INITIALIZER;


static void scmd_multiplex(in_cmd_t command, const plchar_t *payload)
{
   switch (command)
   {

      case IN_FC_ACK:
      {
         pthread_mutex_lock(&ack_mutex);
         pthread_cond_signal(&ack);
         pthread_mutex_unlock(&ack_mutex);
         break;
      }

      case IN_FC_DEBUG:
      {
         fc_debug_t *debug_data = (fc_debug_t *)payload;

         /*
          * read system health data:
          */
         pthread_mutex_lock(&health_data.mutex);
         health_data.voltage = (float)debug_data->analog[9] / 10.0;
         health_data.signal = debug_data->analog[10];
         health_data.current = (float)debug_data->analog[22] / 10.0;
         pthread_mutex_unlock(&health_data.mutex);

         /*
          * read barometer data:
          */
         if (start_baro == 0.0)
         {
            start_baro = debug_data->analog[5] * 0.06;
         }
         threadsafe_float_set(&altitude, debug_data->analog[5] * 0.06 - start_baro);
         
         /*
          * read motor rpm:
          */
         pthread_mutex_lock(&rpm_mutex);
         for (int i = 0; i < platform_motors(); i++)
         {
            rpm[i] = debug_data->analog[i + 26] * 100.0f;
         }
         pthread_mutex_unlock(&rpm_mutex);
         break;
      }


      case IN_FC_EXTERN_CONTROL:
      case IN_FC_DATA_3D:
      case IN_FC_DISPLAY_REQ_KEY:
      case IN_FC_DISPLAY_REQ_MENU:
      case IN_FC_MIXER_QUERY:
      case IN_FC_MIXER_WRITE:
      case IN_FC_PPM_CHANNELS:
      case IN_FC_SETTINGS_REQUEST:
      case IN_FC_SETTINGS_WRITE:
      case IN_FC_ENGINE_TEST:
      case IN_FC_VERSION:
      case IN_M3_YAW:
         break;
   }
}


SIMPLE_THREAD_BEGIN(sread_thread_func)
{
   SIMPLE_THREAD_LOOP_BEGIN
   {
      int status;
      int len;
      in_cmd_t cmd;
      plchar_t payload[sizeof(frame_t)];
      frame_t frame;
      len = serial_read_line(frame, &port);
      if (len < 0)
      {
         LOG(LL_ERROR, "read from serial line failed!");
         continue;
      }
      frame[len] = '\0';
      status = parse_frame(&cmd, payload, frame);

      switch (status)
      {
         case PARSER_NO_ERROR:
            scmd_multiplex(cmd, payload);
            break;

         case PARSER_INVALID_START:
            LOG(LL_DEBUG, "ignored frame with invalid start character!");
            break;

         case PARSER_INVALID_ADDRESS:
            LOG(LL_ERROR, "invalid address in frame!");
            break;

         case PARSER_INVALID_COMMAND:
            //LOG(LL_ERROR, "invalid command in frame!");
            break;

         case PARSER_INVALID_CRC:
            LOG(LL_ERROR, "wrong crc in frame! %s", frame);
            break;

         default:
            LOG(LL_ERROR, "program corruption");
            exit(EXIT_FAILURE);
      }
   }
   SIMPLE_THREAD_LOOP_END
}
SIMPLE_THREAD_END


PERIODIC_THREAD_BEGIN(dreq_thread_func)
{
   const char interval = 10;
   frame_t frame;
   PERIODIC_THREAD_LOOP_BEGIN
   {
      build_frame(frame, OUT_FC_DEBUG, (const plchar_t *)&interval, 1);
      serial_write_line(&port, frame);
   }
   PERIODIC_THREAD_LOOP_END
}
PERIODIC_THREAD_END


int fc_write_motors(mixer_in_t *data)
{
   struct ExternControl ext_ctrl;
   frame_t frame;

   bzero(&ext_ctrl, sizeof(struct ExternControl));
   ext_ctrl.gas = 255.0 * data->gas;
   ext_ctrl.pitch = data->pitch * 127.0;
   ext_ctrl.roll = data->roll * 127.0;
   ext_ctrl.yaw = data->yaw * 127.0;
   ext_ctrl.config = 1;

   build_frame(frame, OUT_FC_EXTERN_CONTROL,
               (const plchar_t *)&ext_ctrl,
               sizeof(struct ExternControl));

   return serial_write_line(&port, frame);
}


static int motors_action(char action, char setting)
{
   frame_t frame;

   char data[2];

   if (action == 'e') /* start motors */
   {
      data[0] = action;
      data[1] = setting;

      build_frame(frame, OUT_FC_MOTORS_ACTION,
                  (const plchar_t *)data, 2);
   }
   else /* stop motors */
   {
      data[0] = 'd';
      build_frame(frame, OUT_FC_MOTORS_ACTION,
                  (const plchar_t *)data, 1);
   }


   int result;
   do
   {
      serial_write_line(&port, frame);
      struct timespec ts;
      clock_gettime(CLOCK_REALTIME, &ts);
      ts = timespec_add_ms(ts, 100);
      pthread_mutex_lock(&ack_mutex);
      result = pthread_cond_timedwait(&ack, &ack_mutex, &ts);
      pthread_mutex_unlock(&ack_mutex);
   }
   while (result == ETIMEDOUT);

   return 0;
}


void fc_start_motors(void)
{
   motors_action('e', setting);
   sleep(5);
}


void fc_stop_motors(void)
{
   motors_action('d', 0);
}


int fc_init(void)
{
   if (!is_initialized)
   {
      config_apply("mk_fc", options);

      int ret;
      is_initialized = 1;
      /* perform initialization once here: */
      if ((ret = serial_open(&port, serial_port, B57600, ICRNL, ICANON, 0)) != 0)
      {
         return ret;
      }
      else
      {
         const struct timespec dreq_period = {0, DREQ_THREAD_TIMEOUT_MS * NSEC_PER_MSEC};
         frame_t frame;
         char data = 1;
         LOG(LL_INFO, "setting up mikrokopter interface");
         build_frame(frame, OUT_NC_REDIRECT_UART, (const plchar_t *)&data, 0);
         serial_write_line(&port, frame);

         rpm = malloc(sizeof(float) * platform_motors());
         for (int i = 0; i < platform_motors(); i++)
         {
            rpm[i] = 0.0f;   
         }
         periodic_thread_start(&dreq_thread, dreq_thread_func, DREQ_THREAD_NAME,
                               DREQ_THREAD_PRIORITY, dreq_period, NULL);

         simple_thread_start(&sread_thread, sread_thread_func, SREAD_THREAD_NAME,
                             SREAD_THREAD_PRIORITY, NULL);

         LOG(LL_INFO, "mikrokopter interface up and running");
      }
   }
   else
   {
      LOG(LL_DEBUG, "mikrokopter interface already running");
   }


   return 0;
}

