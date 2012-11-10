
/*
 * main.c
 *
 *  Created on: 11.06.2010
 *      Author: tobi
 */


#include <signal.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sched.h>
#include <unistd.h>
#include <stdlib.h>
#include <syslog.h>

#include <opcd_params.h>
#include <util.h>
#include <daemon.h>
#include <threadsafe_types.h>
#include <periodic_thread.h>
#include <sclhelper.h>

#include "util/time/interval.h"
#include "util/logger/logger.h"
#include "command/command.h"
#include "util/time/ltime.h"
#include "filters/sliding_avg.h"
#include "model/madgwick_ahrs.h"
#include "model/model.h"
#include "control/control.h"
#include "platform/platform.h"
#include "platform/arcade_quadro.h"
#include "hardware/util/calibration.h"
#include "control/basic/piid.h"
#include "control/position/att_ctrl.h"
#include "filters/sliding_avg.h"


#define REALTIME_PERIOD (0.005)
#define CONTROL_RATIO (2)


static periodic_thread_t realtime_thread;


PERIODIC_THREAD_BEGIN(realtime_thread_func)
{ 
   syslog(LOG_INFO, "initializing core");
   
   /* init SCL subsystem: */
   syslog(LOG_INFO, "initializing signaling and communication link (SCL)");
   if (scl_init("core") != 0)
   {
      syslog(LOG_CRIT, "could not init scl module");
      exit(EXIT_FAILURE);
   }
   
   /* init params subsystem: */
   syslog(LOG_INFO, "initializing opcd interface");
   opcd_params_init("core.", 1);
   
   /* initialize logger: */
   syslog(LOG_INFO, "opening logger");
   if (logger_open() != 0)
   {
      syslog(LOG_CRIT, "could not open logger");
      exit(EXIT_FAILURE);
   }
   syslog(LOG_CRIT, "logger opened");
   sleep(1); /* give scl some time to establish
                a link between publisher and subscriber */

   LOG(LL_INFO, "+------------------+");
   LOG(LL_INFO, "|   core startup   |");
   LOG(LL_INFO, "+------------------+");

   LOG(LL_INFO, "initializing system");

   struct sched_param sp;
   sp.sched_priority = sched_get_priority_max(SCHED_FIFO);
   sched_setscheduler(getpid(), SCHED_FIFO, &sp);

   if (mlockall(MCL_CURRENT | MCL_FUTURE))
   {
      LOG(LL_ERROR, "mlockall() failed");
   }

   LOG(LL_INFO, "initializing model/controller");
   model_init();
   //ctrl_init();

   LOG(LL_INFO, "initializing platform");
   platform_init(arcade_quadro_init);

   LOG(LL_INFO, "initializing command interface");
   cmd_init();

   printf("Press 's' and return to continue\n");
   char start_key = 0;
   while (start_key != 's')
   {
      start_key = getchar();
   }
   
   float channels[MAX_CHANNELS];
   calibration_t rc_cal;
   calibration_init(&rc_cal, MAX_CHANNELS, 400);
   unsigned int timer = 0;
   for (timer = 0; timer < 400; timer++)
   {
      platform_read_rc(channels);
      calibration_sample_bias(&rc_cal, channels);
      msleep(1);
   }
   LOG(LL_INFO, "system up and running");

   /*FILE *fp = fopen("/root/ARCADE_UAV/components/core/temp/log.dat","w");
   if (fp == NULL) printf("ERROR: could not open File");
   fprintf(fp,"gyro_x gyro_y gyro_z motor1 motor2 motor3 motor4 battery_voltage rc_input0 rc_input1 rc_input2 rc_input3 u_ctrl1 u_ctrl2 u_ctrl3\n");*/

   Filter2 filter_out;
   filter2_lp_init(&filter_out, 55.0f, 0.95f, REALTIME_PERIOD, 4);

   piid_t piid;
   piid_init(&piid, REALTIME_PERIOD);
   piid.f_local[0] = 1.00f;

   madgwick_ahrs_t madgwick_ahrs;
   float madgwick_p = 10.0f;
   float madgwick_p_end = 0.01f;
   float madgwick_p_step = 10.0f * REALTIME_PERIOD;
   madgwick_ahrs_init(&madgwick_ahrs, madgwick_p);
   
   sliding_avg_t *avg[3];
   avg[0] = sliding_avg_create(1000, 0.0);
   avg[1] = sliding_avg_create(1000, 0.0);
   avg[2] = sliding_avg_create(1000, -9.81);

   att_ctrl_init();

   interval_t interval;
   interval_init(&interval);
   PERIODIC_THREAD_LOOP_BEGIN
   {
      float dt = interval_measure(&interval);
      /*
       * read sensor values into model input structure:
       */
      model_input_t model_input;
      model_input.dt = dt;

      marg_data_t marg_data;
      gps_data_t gps_data;
      platform_read_marg(&marg_data);
      platform_read_gps(&gps_data);
      platform_read_ultra(&model_input.ultra_z);
      platform_read_baro(&model_input.baro_z);
      float voltage = 16.0f;
      //platform_read_voltage(&voltage);
      
      /* compute estimate of orientation quaternion: */
      madgwick_ahrs_update(&madgwick_ahrs, &marg_data, 11.0, dt);
 
      if (madgwick_p > madgwick_p_end)
      {
         madgwick_p -= madgwick_p_step;
         if (madgwick_p < madgwick_p_end)
         {
            madgwick_p = madgwick_p_end;
            //quat_to_euler(&euler, &madgwick_ahrs.quat);
            /* TODO: initialize pitch/roll/yaw controllers here */
         }
         madgwick_ahrs.beta = madgwick_p;
         continue;
      }

      
      /* compute NED accelerations using quaternion: */
      vec3_t global_acc;
      quat_rot_vec(&global_acc, &marg_data.acc, &madgwick_ahrs.quat);
      for (int i = 0; i < 3; i++)
      {
         global_acc.vec[i] -= sliding_avg_calc(avg[i], global_acc.vec[i]);
      }
      model_input.acc_n = global_acc.x;
      model_input.acc_e = global_acc.y;
      model_input.acc_d = global_acc.z;
      
      /* execute model step: */
      model_state_t model_state;
      model_step(&model_state, &model_input);
     
      /* compute euler angles from quaternion: */
      euler_t euler;
      quat_to_euler(&euler, &madgwick_ahrs.quat);
      float att_dest[2] = {0, 0};
      float att_ctrl[2];
      float att_pos[2] = {euler.pitch, euler.roll};
      att_ctrl_step(att_ctrl, dt, att_pos, att_dest);
      //EVERY_N_TIMES(10, printf("%f %f %f\n", euler.yaw, euler.pitch, euler.roll));

      float gyro_vals[3];
      gyro_vals[0] = marg_data.gyro.x;
      gyro_vals[1] = -marg_data.gyro.y;
      gyro_vals[2] = -marg_data.gyro.z;

    
#if 0

      /*
       * execute controller step:
       */
      ctrl_step(&mixer_in, dt, &model_state);
#endif
      
      /* determine attitude */
      float u_ctrl[3];
      float rc_input[3];

      /* set-up input for basic controller: */
      rc_input[0] = att_ctrl[1]; // -2.0f * rc_roll + 0.0 * mixer_in.roll; // 0.1
      rc_input[1] = -att_ctrl[0]; // 2.0f * rc_pitch - 0.0 * mixer_in.pitch; // 0.1
      rc_input[2] = 0; // -3.0f * rc_yaw + 0.0 * mixer_in.yaw;
      piid.f_local[0] = 0.0; //30.0f * rc_gas;
      
      piid_run(&piid, gyro_vals, rc_input, u_ctrl);
      filter2_run(&filter_out, piid.f_local, piid.f_local);
      piid.f_local[1] /= 30.0;
      piid.f_local[2] /= 30.0;
      piid.f_local[3] = 0.0;
      
      /* here we need to decide if the motors should run: */
      if (0) //rc_ch5 > 0.0 || !rc_dsl_reader_signal_valid())
      {
      }
      int motors_enabled = 1;

      EVERY_N_TIMES(CONTROL_RATIO, piid.int_enable = platform_write_motors(motors_enabled, piid.f_local, voltage));
      //EVERY_N_TIMES(10, printf("%f %f %f\n", piid.f_local[1], piid.f_local[2], piid.f_local[3]));
      //fprintf(fp,"%10.9f %10.9f %10.9f %d %d %d %d %6.4f %6.4f %6.4f %6.4f %6.4f %10.9f %10.9f %10.9f\n",gyro_vals[0],gyro_vals[1],gyro_vals[2],i2c_buffer[0],i2c_buffer[1],i2c_buffer[2],i2c_buffer[3],voltage,controller.f_local[0],rc_input[0], rc_input[1], rc_input[2], u_ctrl[0], u_ctrl[1], u_ctrl[2]);
      //mixer_in_t mixer_in;
   }
   PERIODIC_THREAD_LOOP_END
}
PERIODIC_THREAD_END


void _cleanup(void)
{
   static int killing = 0;
   if (!killing)
   {
      killing = 1;
      LOG(LL_INFO, "system shutdown by user");
      sleep(1);
      kill(0, 9);
   }
}


void _main(int argc, char *argv[])
{
   (void)argc;
   (void)argv;
   const struct timespec realtime_period = {0, REALTIME_PERIOD * 1000 * NSEC_PER_MSEC};
   periodic_thread_start(&realtime_thread, realtime_thread_func, "realtime_thread", 99, realtime_period, NULL);
   while (1)
   {
      sleep(10000);   
   }
}


int main(int argc, char *argv[])
{
   _main(argc, argv);
   daemonize("/var/run/core.pid", _main, _cleanup, argc, argv);
   return 0;
}


