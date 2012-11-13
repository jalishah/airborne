
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
#include "control/basic/feed_forward.h"
#include "control/position/att_ctrl.h"
#include "filters/sliding_avg.h"
#include "hardware/util/calibration.h"
#include "hardware/util/gps_util.h"


typedef union
{
   struct
   {
      float gas; /* [N] */
      float roll; /* rad/s */
      float pitch; /* rad/s */
      float yaw; /* rad/s */
   };
   float vec[4];
}
f_local_t;


typedef enum
{
   CM_DISARMED,  /* motors are completely disabled for safety */
   CM_MANUAL,    /* direct remote control without any position control
                    if the RC signal is lost, altitude stabilization is enabled and the GPS setpoint is reset */
   CM_GUIDED,    /* pilot controls global speed vector using right stick, gas is "vario-height"  */
   CM_SAFE_AUTO, /* device works autonomously, stick movements disable autonomous operation with some hysteresis */
   CM_FULL_AUTO  /* remote control interface is unused */
}
control_mode_t;


control_mode_t control_mode = CM_DISARMED;


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

   LOG(LL_INFO, "core initializing");

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
   /*calibration_t rc_cal;
   calibration_init(&rc_cal, MAX_CHANNELS, 400);
   unsigned int timer = 0;
   for (timer = 0; timer < 400; timer++)
   {
      platform_read_rc(channels);
      calibration_sample_bias(&rc_cal, channels);
      msleep(1);
   }*/
   LOG(LL_INFO, "system up and running");

   /*FILE *fp = fopen("/root/ARCADE_UAV/components/core/temp/log.dat","w");
   if (fp == NULL) printf("ERROR: could not open File");
   fprintf(fp,"gyro_x gyro_y gyro_z motor1 motor2 motor3 motor4 battery_voltage rc_input0 rc_input1 rc_input2 rc_input3 u_ctrl1 u_ctrl2 u_ctrl3\n");*/

   Filter2 filter_out;
   filter2_lp_init(&filter_out, 55.0f, 0.95f, REALTIME_PERIOD, 4);

   feed_forward_t feed_forward;
   feed_forward_init(&feed_forward, REALTIME_PERIOD);
   piid_t piid;
   piid_init(&piid, REALTIME_PERIOD);

   madgwick_ahrs_t madgwick_ahrs;
   madgwick_ahrs_init(&madgwick_ahrs, 10.0f, 10.0f * REALTIME_PERIOD, 0.01f);
   quat_t start_quat;
 
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
      model_input.dx = gps_rel_data.dx;
      model_input.dy = gps_rel_data.dy;
      platform_read_ultra(&model_input.ultra_z);
      platform_read_baro(&model_input.baro_z);
      int rc_sig_valid = (platform_read_rc(channels) == 0);

      float voltage;
      platform_read_voltage(&voltage);

      /* compute estimate of orientation quaternion: */
      int ahrs_state = madgwick_ahrs_update(&madgwick_ahrs, &marg_data, 1.0, dt);
      if (ahrs_state < 0)
         continue;
      else if (ahrs_state == 0)
      {
         start_quat = madgwick_ahrs.quat;
         LOG(LL_DEBUG, "initial quaternion orientation estimate: %f %f %f %f", start_quat.q0, start_quat.q1, start_quat.q2, start_quat.q3);
      }

      /* compute NED accelerations using quaternion: */
      quat_rot_vec(&model_input.acc, &marg_data.acc, &madgwick_ahrs.quat);
      
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
      rc_input[0] = att_ctrl[1] + 2.0f * channels[CH_ROLL]; /* [rad/s] */
      rc_input[1] = -att_ctrl[0] + 2.0f * channels[CH_PITCH]; /* [rad/s] */
      rc_input[2] = 3.0f * channels[CH_YAW]; /* [rad/s] */
 
      /* run feed-forward and piid controller: */
      float u_ctrl[3];
      float gyro_vals[3];
      gyro_vals[0] = marg_data.gyro.x;
      gyro_vals[1] = -marg_data.gyro.y;
      gyro_vals[2] = -marg_data.gyro.z;
      feed_forward_run(&feed_forward, u_ctrl, rc_input);
      piid_run(&piid, u_ctrl, gyro_vals, rc_input);
      

      /* fill and filter 4d output signals: */
      f_local_t f_local;
      f_local.gas = 30.0f * channels[CH_GAS]; /* [N] */
      FOR_N(i, 3)
         f_local.vec[i + 1] = u_ctrl[i];
      filter2_run(&filter_out, f_local.vec, f_local.vec);

      /* here we need to decide whether the motors should run: */
      int motors_enabled;
      if (channels[CH_SWITCH] < 0.5 && rc_sig_valid)
      {
         motors_enabled = 1;
      }
      else
      {
         f_local.gas = 1.0f;   /* 1 newton overall thrust */
         f_local.roll = 0.0f;  /*    no .. */
         f_local.pitch = 0.0f; /* .. additional .. */
         f_local.yaw = 0.0f;   /* .. torques */
         motors_enabled = 1;
      }
      motors_enabled = 0;

      EVERY_N_TIMES(CONTROL_RATIO, piid.int_enable = platform_write_motors(motors_enabled, f_local.vec, voltage));
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


