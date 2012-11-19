
/*
   ARCADE Airborne Main Program

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
#include "estimators/ahrs.h"
#include "estimators/pos.h"
#include "control/control.h"
#include "platform/platform.h"
#include "platform/arcade_quadro.h"
#include "control/basic/piid.h"
#include "control/basic/feed_forward.h"
#include "filters/sliding_avg.h"
#include "hardware/util/calibration.h"
#include "hardware/util/gps_util.h"
#include "control/position/navi.h"
#include "control/position/att_ctrl.h"
#include "control/position/z_ctrl.h"
#include "control/position/yaw_ctrl.h"
#include "control/speed/xy_speed.h"


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


enum
{
   CM_DISARMED,  /* motors are completely disabled for safety */
   CM_MANUAL,    /* direct remote control without any position control
                    if the RC signal is lost, altitude stabilization is enabled and the GPS setpoint is reset */
   CM_SAFE_AUTO, /* device works autonomously, stick movements disable autonomous operation with some hysteresis */
   CM_FULL_AUTO  /* remote control interface is unused */
}
mode = CM_SAFE_AUTO; //CM_SAFE_AUTO;



#define REALTIME_PERIOD (0.005)
#define CONTROL_RATIO (2)


#define RC_PITCH_ROLL_STICK_P 2.0f
#define RC_YAW_STICK_P 3.0f


static periodic_thread_t realtime_thread;


static int gyro_moved(vec3_t *gyro)
{
   FOR_N(i, 3)
   {
      if (fabs(gyro->vec[i]) >  0.15)
      {
         return 1;   
      }
   }
   return 0;
}


void die(void)
{
   static int killing = 0;
   if (!killing)
   {
      killing = 1;
      LOG(LL_INFO, "shutting down");
      sleep(1);
      kill(0, 9);
   }
}


PERIODIC_THREAD_BEGIN(realtime_thread_func)
{ 
   syslog(LOG_INFO, "initializing core");
   
   /* init SCL subsystem: */
   syslog(LOG_INFO, "initializing signaling and communication link (SCL)");
   if (scl_init("core") != 0)
   {
      syslog(LOG_CRIT, "could not init scl module");
      die();
   }
   
   /* init params subsystem: */
   syslog(LOG_INFO, "initializing opcd interface");
   opcd_params_init("core.", 1);
   
   /* initialize logger: */
   syslog(LOG_INFO, "opening logger");
   if (logger_open() != 0)
   {
      syslog(LOG_CRIT, "could not open logger");
      die();
   }
   syslog(LOG_CRIT, "logger opened");
   sleep(1); /* give scl some time to establish
                a link between publisher and subscriber */

   LOG(LL_INFO, "core initializing");

   
   LOG(LL_INFO, "setting maximum CPU clock");
   if (system("echo performance > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor") != 0)
   {
      LOG(LL_ERROR, "failed");
      die();
   }
   
   LOG(LL_INFO, "setting up real-time scheduling");
   struct sched_param sp;
   sp.sched_priority = sched_get_priority_max(SCHED_FIFO);
   sched_setscheduler(getpid(), SCHED_FIFO, &sp);

   if (mlockall(MCL_CURRENT | MCL_FUTURE) != 0)
   {
      LOG(LL_ERROR, "mlockall() failed");
      die();
   }

   LOG(LL_INFO, "initializing platform");
   if (platform_init(arcade_quadro_init) < 0)
   {
      LOG(LL_ERROR, "could not initialize platform");
      die();
   }

   LOG(LL_INFO, "initializing model/controller");
   pos_init();
   xy_speed_ctrl_init();
   att_ctrl_init();
   yaw_ctrl_init();
   z_ctrl_init(platform_param()->mass_kg * 10.0f);
   navi_init();

   LOG(LL_INFO, "initializing command interface");
   cmd_init();

   float channels[MAX_CHANNELS];

   /*FILE *fp = fopen("/root/ARCADE_UAV/components/core/temp/log.dat","w");
   if (fp == NULL) printf("ERROR: could not open File");
   fprintf(fp,"gyro_x gyro_y gyro_z motor1 motor2 motor3 motor4 battery_voltage rc_input0 rc_input1 rc_input2 rc_input3 u_ctrl1 u_ctrl2 u_ctrl3\n");*/

   calibration_t gyro_cal;
   cal_init(&gyro_cal, 3, 1000);
   
   Filter2 filter_out;
   filter2_lp_init(&filter_out, 55.0f, 0.95f, REALTIME_PERIOD, 4);

   feed_forward_t feed_forward;
   feed_forward_init(&feed_forward, REALTIME_PERIOD);
   piid_t piid;
   piid_init(&piid, REALTIME_PERIOD);

   ahrs_t ahrs;
   ahrs_init(&ahrs, 10.0f, 2.0f * REALTIME_PERIOD, 0.03f);
   quat_t start_quat;
   
   /* perform initial marg reading to acquire an initial orientation guess: */
   marg_data_t marg_data;
   /*platform_read_marg(&marg_data);
     quaternion_init(&ahrs.quat, &marg_data.acc, &marg_data.mag);*/
 
   gps_util_t gps_util;
   gps_util_init(&gps_util);
   gps_rel_data_t gps_rel_data = {0.0, 0.0, 0.0};

   LOG(LL_INFO, "entering main loop");
   interval_t interval;
   interval_init(&interval);
   PERIODIC_THREAD_LOOP_BEGIN
   {
      /*******************************************
       * read sensor data and calibrate sensors: *
       *******************************************/

      float dt = interval_measure(&interval);
      pos_in_t pos_in;
      pos_in.dt = dt;
      platform_read_marg(&marg_data);
      if (cal_sample_apply(&gyro_cal, (float *)&marg_data.gyro.vec) == 0 && gyro_moved(&marg_data.gyro))
      {
         LOG(LL_ERROR, "gyro moved during calibration, retrying");
         sleep(1);
         cal_reset(&gyro_cal);
      }
      
      gps_data_t gps_data;
      platform_read_gps(&gps_data);
      gps_util_update(&gps_rel_data, &gps_util, &gps_data);
      pos_in.dx = gps_rel_data.dx;
      pos_in.dy = gps_rel_data.dy;
      platform_read_ultra(&pos_in.ultra_z);
      platform_read_baro(&pos_in.baro_z);
      int rc_sig_valid = (platform_read_rc(channels) == 0);
      float voltage = 16.0f;
      platform_read_voltage(&voltage);


      /********************************
       * perform sensor data fusion : *
       ********************************/

      euler_t euler;
      int ahrs_state = ahrs_update(&ahrs, &marg_data, 1.0, dt);
      if (ahrs_state < 0)
      {
         continue;
      }
      else
      {
         /* compute euler angles from quaternion: */
         quat_to_euler(&euler, &ahrs.quat);
         if (ahrs_state == 1)
         {
            start_quat = ahrs.quat;
            LOG(LL_INFO, "system initialized");
            LOG(LL_DEBUG, "initial orientation estimate; yaw: %f pitch: %f roll: %f", euler.yaw, euler.pitch, euler.roll);
         }
      }

      /* compute NEU accelerations using quaternion: */
      quat_rot_vec(&pos_in.acc, &marg_data.acc, &ahrs.quat);
      pos_in.acc.z *= -1.0f; /* <-- transform from NED to NEU frame */
      
      /* compute next 3d position estimate: */
      pos_t pos_estimate;
      pos_update(&pos_estimate, &pos_in);

      //EVERY_N_TIMES(10, printf("%f %f %f %f\n", pos_estimate.x.pos, pos_estimate.y.pos, pos_estimate.ultra_z.pos, pos_estimate.baro_z.pos));

      /******************************************************
       * run higher level controllers including navigation  *
       * results are stored in auto_stick:                  *
       ******************************************************/

      ctrl_out_t auto_stick;
   
      /* run yaw controller: */
      float yaw_err;
      auto_stick.yaw = yaw_ctrl_step(&yaw_err, euler.yaw, marg_data.gyro.z, dt);

      /* run z controller: */
      float z_err;
      auto_stick.gas = z_ctrl_step(&z_err, pos_estimate.ultra_z.pos,
                                   pos_estimate.baro_z.pos, pos_estimate.baro_z.speed, dt);

      int xy_speed_control = 1;
      vec2_t speed_sp;
      if (xy_speed_control)
      {
         /* direct speed control */
         speed_sp.x = 0.0f;
         speed_sp.y = 0.0f;
      }
      else
      {
         /* run xy navigation controller: */
         vec2_t pos_vec = {{pos_estimate.x.pos, pos_estimate.y.pos}};
         navi_run(&speed_sp, &pos_vec, dt);
      }
      /* run speed vector controller: */
      vec2_t speed_vec = {{pos_estimate.x.speed, pos_estimate.y.speed}};
      vec2_t pitch_roll_sp;
      xy_speed_ctrl_run(&pitch_roll_sp, &speed_sp, &speed_vec, euler.yaw);
   
      /* run attitude controller: */
      vec2_t pitch_roll_ctrl;
      vec2_t pitch_roll = {{euler.pitch, euler.roll}};
      pitch_roll_sp.x = 0;
      pitch_roll_sp.y = 0;
      att_ctrl_step(&pitch_roll_ctrl, dt, &pitch_roll, &pitch_roll_sp);
      auto_stick.pitch = pitch_roll_ctrl.x;
      auto_stick.roll = -pitch_roll_ctrl.y;


      /*************************************
       * run basic stabilizing controller: *
       *************************************/
      
      /* set up controller inputs: */
      float ff_piid_sp[3] = {0.0f, 0.0f, 0.0f};
      f_local_t f_local = {{0.0f, 0.0f, 0.0f, 0.0f}};
      if (mode >= CM_SAFE_AUTO)
      {
         ff_piid_sp[0] = auto_stick.roll;
         ff_piid_sp[1] = auto_stick.pitch;
         ff_piid_sp[2] = 0.0f; //auto_stick.yaw;
         f_local.gas = 8.0f; //auto_stick.gas * platform_param()->max_thrust_n;
      }
      if (rc_sig_valid && mode != CM_FULL_AUTO)
      {
         /* mix in rc signals for "emergency takeover": */
         ff_piid_sp[0] += RC_PITCH_ROLL_STICK_P * channels[CH_ROLL];
         ff_piid_sp[1] += RC_PITCH_ROLL_STICK_P * channels[CH_PITCH];
         ff_piid_sp[2] += RC_YAW_STICK_P * channels[CH_YAW];

         /* adjust thrust by gas stick: */
         if (   (mode == CM_SAFE_AUTO && channels[CH_GAS] < f_local.gas)
             || (mode == CM_MANUAL))
         {
            f_local.gas = channels[CH_GAS] * platform_param()->max_thrust_n;
         }
      }

      /* run feed-forward system and stabilizing PIID controller: */
      float gyro_vals[3] = {marg_data.gyro.x, -marg_data.gyro.y, -marg_data.gyro.z};
      feed_forward_run(&feed_forward, &f_local.vec[1], ff_piid_sp);
      piid_run(&piid, &f_local.vec[1], gyro_vals, ff_piid_sp);
      filter2_run(&filter_out, f_local.vec, f_local.vec);
 
 
      /********************
       * actuator output: *
       ********************/
 
      int motors_enabled;
      if (mode == CM_DISARMED)
      {
         motors_enabled = 0;
      }
      else if (mode != CM_FULL_AUTO)
      {
         motors_enabled = channels[CH_SWITCH] < 0.5 && rc_sig_valid;
      }
      else
      {
         motors_enabled = 1;
      }
      EVERY_N_TIMES(CONTROL_RATIO, piid.int_enable = platform_write_motors(motors_enabled, f_local.vec, voltage));
      
      
      //EVERY_N_TIMES(10, printf("%f\t\t %f\t\t %f\n", piid.f_local[1], piid.f_local[2], piid.f_local[3]));
      //fprintf(fp,"%10.9f %10.9f %10.9f %d %d %d %d %6.4f %6.4f %6.4f %6.4f %6.4f %10.9f %10.9f %10.9f\n",gyro_vals[0],gyro_vals[1],gyro_vals[2],i2c_buffer[0],i2c_buffer[1],i2c_buffer[2],i2c_buffer[3],voltage,controller.f_local[0],rc_input[0], rc_input[1], rc_input[2], u_ctrl[0], u_ctrl[1], u_ctrl[2]);
   }
   PERIODIC_THREAD_LOOP_END
}
PERIODIC_THREAD_END



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
   daemonize("/var/run/core.pid", _main, die, argc, argv);
   return 0;
}


