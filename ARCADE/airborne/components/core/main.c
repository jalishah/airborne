
/*
   ARCADE airborne main program - implementation

   Copyright (C) 2012 Tobias Simon, Ilmenau University of Technology

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
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
#include "control/basic/piid.h"
#include "control/position/att_ctrl.h"
#include "filters/sliding_avg.h"
#include "hardware/util/calibration.h"
#include "hardware/util/gps_util.h"


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

   madgwick_ahrs_t madgwick_ahrs;
   madgwick_ahrs_init(&madgwick_ahrs, 10.0f, 10.0f * REALTIME_PERIOD, 0.01f);
   
   float avg_init[3] = {0.0f, 0.0f, -9.81f};
   sliding_avg_t *avg[3];
   FOR_N(i, 3)
      avg[i] = sliding_avg_create(1000, avg_init[i]);

   att_ctrl_init();
   
   gps_util_t gps_util;
   gps_util_init(&gps_util);
   gps_rel_data_t gps_rel_data = {0.0, 0.0, 0.0};
   
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
      platform_read_marg(&marg_data);
      gps_data_t gps_data;
      platform_read_gps(&gps_data);
      gps_util_update(&gps_rel_data, &gps_util, &gps_data);
      platform_read_ultra(&model_input.ultra_z);
      platform_read_baro(&model_input.baro_z);
      int rc_sig_valid = (platform_read_rc(channels) == 0);

      float voltage;
      platform_read_voltage(&voltage);

      /* compute estimate of orientation quaternion: */
      int ahrs_state = madgwick_ahrs_update(&madgwick_ahrs, &marg_data, 1.0, dt);
      if (ahrs_state < 0)
         continue;
      
      /* compute NED accelerations using quaternion: */
      vec3_t global_acc;
      quat_rot_vec(&global_acc, &marg_data.acc, &madgwick_ahrs.quat);
      FOR_N(i, 3)
         global_acc.vec[i] -= sliding_avg_calc(avg[i], global_acc.vec[i]);
      model_input.acc_n = global_acc.x;
      model_input.acc_e = global_acc.y;
      model_input.acc_d = global_acc.z;
      
      /* execute kalman filters for position estimate: */
      model_state_t model_state;
      model_step(&model_state, &model_input);
 
      /* compute euler angles from quaternion: */
      euler_t euler;
      quat_to_euler(&euler, &madgwick_ahrs.quat);
      float att_dest[2] = {0, 0};
      float att_ctrl[2];
      float att_pos[2] = {euler.pitch, euler.roll};
      att_ctrl_step(att_ctrl, dt, att_pos, att_dest);

      /* set-up input for basic controller: */
      float rc_input[3];
      rc_input[0] = att_ctrl[1] + 2.0f * channels[CH_ROLL];
      rc_input[1] = -att_ctrl[0] + 2.0f * channels[CH_PITCH];
      rc_input[2] = 3.0f * channels[CH_YAW];
      piid.f_local[0] = 30.0f * channels[CH_GAS];
 
      /* run feed-forward and piid controller: */
      float u_ctrl[3];
      float gyro_vals[3];
      gyro_vals[0] = marg_data.gyro.x;
      gyro_vals[1] = -marg_data.gyro.y;
      gyro_vals[2] = -marg_data.gyro.z;
      piid_run(&piid, gyro_vals, rc_input, u_ctrl);
      filter2_run(&filter_out, piid.f_local, piid.f_local);

      /* here we need to decide whether the motors should run: */
      int motors_enabled;
      if (channels[CH_SWITCH] < 0.5 && rc_sig_valid)
      {
         motors_enabled = 1;
      }
      else
      {
         piid.f_local[0] = 1.0f; /* 1 newton overall thrust */
         piid.f_local[1] = 0.0f; /*    no .. */
         piid.f_local[2] = 0.0f; /* .. additional .. */
         piid.f_local[3] = 0.0f; /* .. torques */
         motors_enabled = 1;
      }

      EVERY_N_TIMES(CONTROL_RATIO, piid.int_enable = platform_write_motors(motors_enabled, piid.f_local, voltage));
      //EVERY_N_TIMES(10, printf("%f\t\t %f\t\t %f\n", piid.f_local[1], piid.f_local[2], piid.f_local[3]));
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


