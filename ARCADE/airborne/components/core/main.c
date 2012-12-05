
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
#include <threads/periodic_thread.h>
#include <threadsafe_types.h>
#include <sclhelper.h>

#include "util/time/interval.h"
#include "util/logger/logger.h"
#include "command/command.h"
#include "util/time/ltime.h"
#include "estimators/ahrs.h"
#include "estimators/pos.h"
#include "control/control.h"
#include "platform/platform.h"
#include "platform/arcade_quadro.h"
#include "control/basic/piid.h"
#include "control/basic/feed_forward.h"
#include "hardware/util/calibration.h"
#include "hardware/util/gps_util.h"
#include "hardware/util/mag_decl.h"
#include "flight_detect.h"
#include "control/position/navi.h"
#include "control/position/att_ctrl.h"
#include "control/position/z_ctrl.h"
#include "control/position/yaw_ctrl.h"
#include "control/speed/xy_speed.h"
#include "motostate.h"
#include "calpub.h"


static int calibrate = 0;


void main_calibrate(int enabled)
{
   calibrate = enabled;   
}


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
  MS_EXCLUDED, /* writing the motor controllers is not performed; this might result in a reduced loop time, but is completely safe */
  MS_ZERO, /* motors are always written with 0, thus disabled (bus errors might start them, careful when working on bus drivers!) */
  MS_ENABLED /* motors are armed */
}
motor_safety = MS_EXCLUDED;


enum
{
   CM_MANUAL,    /* direct remote control without any position control
                    if the RC signal is lost, altitude stabilization is enabled and the GPS setpoint is reset */
   CM_SAFE_AUTO, /* device works autonomously, stick movements disable autonomous operation with some hysteresis */
   CM_FULL_AUTO  /* remote control interface is unused */
}
mode = CM_MANUAL; //SAFE_AUTO;



int auto_xy = 0;
int auto_yaw = 0;
int auto_z = 0;


enum
{
   M_ATT_REL, /* relative attitude control (aka heading hold, axis lock,...) */
   M_ATT_ABS, /* absolute attitude control (aka acc-mode) */
   M_ATT_GPS_SPEED /* stick defines GPS speed for local coordinate frame */
}
m_att_mode = M_ATT_ABS;


#define REALTIME_PERIOD (0.01)


#define RC_PITCH_ROLL_STICK_P 2.0f
#define RC_YAW_STICK_P 3.0f

#define STICK_GPS_SPEED_MAX 5.0


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



static void _main(int argc, char *argv[])
{
   (void)argc;
   (void)argv;
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

   if (nice(-20) == -1)
   {
      LOG(LL_ERROR, "could not renice process");
      die();
   }

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

   calpub_init();

   LOG(LL_INFO, "initializing model/controller");
   pos_init();
   xy_speed_ctrl_init();
   att_ctrl_init();
   yaw_ctrl_init();
   z_ctrl_init(platform_param()->mass_kg * 10.0f / platform_param()->max_thrust_n);
   navi_init();

   LOG(LL_INFO, "initializing command interface");
   cmd_init();

   motostate_init(0.21f, 0.15f, 0.1f);

   FILE *fp = fopen("/root/MOBICOM/build/temp/log.dat","w");
   if (fp == NULL) 
   {
      printf("ERROR: could not open File");
      die();
   }
   fprintf(fp, "dt " /* #1 */
               "gyro_x gyro_y gyro_z " /* #2 */
               "acc_x acc_y acc_z " /* #3 */
               "mag_x mag_y mag_z " /* #4 */
               "q0 q1 q2 q3 " /* #5 */
               "yaw pitch roll " /* #6 */
               "acc_e acc_n acc_u " /* #7 */
               "raw_e raw_n raw_ultra_u raw_baro_u " /* #8 */
               "pos_e pos_n pos_ultra_u pos_baro_u " /* #9 */
               "speed_e pos_n speed_ultra_u pos_ultra_u " /* #10 */
               "yaw_sp pitch_sp roll_sp\n"); /* #11 */


   calibration_t gyro_cal;
   cal_init(&gyro_cal, 3, 500);
   
   feed_forward_t feed_forward;
   feed_forward_init(&feed_forward, REALTIME_PERIOD);
   piid_t piid;
   piid_init(&piid, REALTIME_PERIOD);

   ahrs_t ahrs;
   ahrs_init(&ahrs, 10.0f, 2.0f * REALTIME_PERIOD, 0.02f);
   quat_t start_quat;
   
   gps_util_t gps_util;
   gps_util_init(&gps_util);

   flight_detect_init(9, 10000, 0.0f, NULL);
   
   LOG(LL_INFO, "entering main loop");
   interval_t interval, gyro_move_interval;
   interval_init(&interval);
   interval_init(&gyro_move_interval);
   //Filter1 dt_filter;
   //filter1_lp_init(&dt_filter, 0.01, REALTIME_PERIOD, 1);
   //dt_filter.z[0] = REALTIME_PERIOD;

   periodic_thread_t _thread; periodic_thread_t *thread = &_thread;
   _thread.name = "mainloop"; _thread.running = 1;
   _thread.periodic_data.period.tv_sec = 0;
   _thread.periodic_data.period.tv_nsec = NSEC_PER_SEC * REALTIME_PERIOD;
   float mag_bias = 0.0f;
   float mag_decl = 0.0f;
   PERIODIC_THREAD_LOOP_BEGIN
   {
      
      /*******************************************
       * read sensor data and calibrate sensors: *
       *******************************************/
      float dt = interval_measure(&interval);
      pos_in_t pos_in;
      pos_in.dt = dt;
      
      int marg_valid = platform_read_marg(&marg_data) == 0;
      if (!marg_valid)
         continue;
      if (cal_sample_apply(&gyro_cal, &marg_data.gyro.vec[0]) == 0 && gyro_moved(&marg_data.gyro))
      {
         if (interval_measure(&gyro_move_interval) > 1.0)
            LOG(LL_ERROR, "gyro moved during calibration, retrying");
         cal_reset(&gyro_cal);
      }

      if (calibrate)
      {
         EVERY_N_TIMES(10, calpub_send(&marg_data));
         continue;
      }
      
      gps_data_t gps_data;
      int gps_valid = platform_read_gps(&gps_data) == 0;
      if (gps_data.fix < FIX_2D)
      {
         gps_valid = 0;
      }
      else
      {
         gps_valid = 1;
         gps_util_update(&gps_rel_data, &gps_util, &gps_data);
         pos_in.dx = gps_rel_data.dx;
         pos_in.dy = gps_rel_data.dy;
      }
      if (gps_valid)
         ONCE(LOG(LL_ERROR, "declination lookup yields: %f", mag_decl);
              mag_decl = mag_decl_lookup(gps_data.lat, gps_data.lon));
      
      int ultra_valid = platform_read_ultra(&pos_in.ultra_z) == 0;
      int baro_valid = platform_read_baro(&pos_in.baro_z) == 0;
      float channels[MAX_CHANNELS];
      int rc_sig_valid = platform_read_rc(channels) == 0;

      float fd_in[9] = {marg_data.gyro.x, marg_data.gyro.y, marg_data.gyro.z,
                        marg_data.acc.x, marg_data.acc.y, marg_data.acc.z,
                        marg_data.mag.x, marg_data.mag.y, marg_data.mag.z};

      int flying = flight_detect(fd_in);

      /* run speed vector controller: */
      vec2_t speed_vec = {{pos_estimate.x.speed, pos_estimate.y.speed}};
      xy_speed_ctrl_run(&pitch_roll_sp, &speed_sp, &speed_vec, euler.yaw);
      FOR_N(i, 2) pitch_roll_sp.vec[i] = sym_limit(pitch_roll_sp.vec[i], 0.2);

      /* run attitude controller: */
      vec2_t pitch_roll_ctrl;
      vec2_t pitch_roll = {{euler.pitch, euler.roll}};
      if (manual_mode == M_ATT_ABS)
      {
         /* interpret sticks as pitch and roll setpoints: */
         pitch_roll_sp.x = -1.0f * channels[CH_PITCH];
         pitch_roll_sp.y = 1.0f * channels[CH_ROLL];
      }
      vec2_t att_err;
      vec2_t pitch_roll_speed = {{marg_data.gyro.y, marg_data.gyro.x}};
      att_ctrl_step(&pitch_roll_ctrl, &att_err, dt, &pitch_roll, &pitch_roll_speed, &pitch_roll_sp);
      auto_stick.pitch = pitch_roll_ctrl.x;
      auto_stick.roll = pitch_roll_ctrl.y;

      /*************************************
       * run basic stabilizing controller: *
       *************************************/

      float ff_piid_sp[3] = {0.0f, 0.0f, 0.0f};
      f_local_t f_local = {{0.0f, 0.0f, 0.0f, 0.0f}};
      float norm_gas = 0.0f; /* interval: [0.0 ... 1.0] */

      if (mode >= CM_SAFE_AUTO || (mode == CM_MANUAL && manual_mode == M_ATT_ABS))
      {
         ff_piid_sp[0] = auto_stick.roll;
         ff_piid_sp[1] = -auto_stick.pitch;
         ff_piid_sp[2] = 0; //auto_stick.yaw;
         norm_gas = auto_stick.gas;
      }
      if (rc_sig_valid && channels[CH_SWITCH] > 0.5 && mode != CM_FULL_AUTO)
      {
         vec2_t pitch_roll_sp = {{0.0f, 0.0f}};
         vec2_t speed_sp;
         if (m_att_mode == M_ATT_GPS_SPEED)
         {
            /* direct speed control via pitch/roll stick: */
            speed_sp.x = STICK_GPS_SPEED_MAX * channels[CH_PITCH] * cosf(euler.yaw);
            speed_sp.y = STICK_GPS_SPEED_MAX * channels[CH_ROLL] * sinf(euler.yaw);
         }
         ff_piid_sp[2] -= RC_YAW_STICK_P * channels[CH_YAW];

         /* limit thrust using "gas" stick: */
         float stick_gas = channels[CH_GAS];
         if (   (/*mode == CM_SAFE_AUTO && */stick_gas < norm_gas)
             /*|| (mode == CM_MANUAL)*/)
         {
            norm_gas = stick_gas;
         }
      }
      /* scale relative gas value to absolute newtons: */
      f_local.gas = norm_gas * platform_param()->max_thrust_n;

 
      /************************************************************
       * run feed-forward system and stabilizing PIID controller: *
       ************************************************************/
 
      float gyro_vals[3] = {marg_data.gyro.x, -marg_data.gyro.y, -marg_data.gyro.z};
      feed_forward_run(&feed_forward, &f_local.vec[1], piid_sp);
      piid_run(&piid, &f_local.vec[1], gyro_vals, piid_sp);
 
 
      /********************
       * actuator output: *
       ********************/
 
      /* requirements specification for take-off: */
      int common_require = marg_valid && voltage_valid && ultra_valid;
      int manual_require = common_require && (m_att_mode == M_GPS_SPEED ? gps_valid : 1) && rc_sig_valid && channels[CH_SWITCH] > 0.5;
      int full_auto_require = common_require && baro_valid && gps_valid;
      int safe_auto_require = full_auto_require && rc_sig_valid && channels[CH_SWITCH] > 0.5;
      
      int satisfied = 0; /* initial value applies to CM_DISABLED */
      if (mode == CM_MANUAL)
         satisfied = manual_require;
      else if (mode == CM_FULL_AUTO)
         satisfied = full_auto_require;
      else if (mode == CM_SAFE_AUTO)
         satisfied = safe_auto_require;

      /* start motors if requirements are met AND conditions apply;
       * stopping the motors does not depend on requirements: */
      motostate_update(pos_in.ultra_z, f_local.gas / platform_param()->max_thrust_n, dt, satisfied);
      if (!motostate_controllable())
      {
         //memset(&f_local, 0, sizeof(f_local)); /* all moments are 0 / minimum motor RPM */
         //piid_reset(&piid); /* reset piid integrators so that we can move the device manually */
         /* TODO: also reset higher-level controllers */
      }
      //memset(&f_local, 0, sizeof(f_local)); /* all moments are 0 / minimum motor RPM */

      /* write forces to motors: */
      piid.int_enable = platform_write_motors(/*motostate_enabled()*/ 0, f_local.vec, voltage);
 
      //float fdt; filter1_run(&dt_filter, &dt, &fdt);

#if 0
      fprintf(fp,
              "%f "          /* #1  time step */ 
              "%f %f %f "    /* #2  gyroscope measurements */
              "%f %f %f "    /* #3  accelerometer measurements */
              "%f %f %f "    /* #4  magnetometer sensor measurements */
              "%f %f %f %f " /* #5  orientation quaternion estimate (uses #1-4) */
              "%f %f %f "    /* #6  euler angles estimate (uses #5) */
              "%f %f %f "    /* #7  global accelerations in NEU frame (uses #1, #3, #5) */
              "%f %f %f %f " /* #8  raw values for GPS x, y and ultra_z, baro_z */
              "%f %f %f %f " /* #9  position estimates for GPS x, y and ultra_z, baro_z (uses #1, #7, #8) */
              "%f %f %f %f " /* #10 speed estimates for GPS x, y and ultra_z, baro_z (uses #1, #7, #8) */
              "%f %f %f\n",  /* #11 setpoints for yaw, pitch, roll */
              dt, /* #1 */
              marg_data.gyro.x, marg_data.gyro.y, marg_data.gyro.z, /* #2 */
              marg_data.acc.x, marg_data.acc.y, marg_data.acc.z, /* #3 */
              marg_data.mag.x, marg_data.mag.y, marg_data.mag.z, /* #4 */
              ahrs.quat.q0, ahrs.quat.q1, ahrs.quat.q2, ahrs.quat.q3, /* #5 */
              euler.yaw, euler.pitch, euler.roll, /* #6 */
              pos_in.acc.x, pos_in.acc.y, pos_in.acc.z, /* #7 */
              pos_in.dx, pos_in.dy, pos_in.ultra_z, pos_in.baro_z, /* #8 */
              pos_estimate.x.pos, pos_estimate.y.pos, pos_estimate.ultra_z.pos, pos_estimate.baro_z.pos, /* #9 */
              pos_estimate.x.speed, pos_estimate.y.speed, pos_estimate.ultra_z.speed, pos_estimate.baro_z.speed, /* #10 */
              0.0f, pitch_roll_sp.x, pitch_roll_sp.y); /* #11 */
   
#endif
   }
   PERIODIC_THREAD_LOOP_END
}


int main(int argc, char *argv[])
{
   _main(argc, argv);
   daemonize("/var/run/core.pid", _main, die, argc, argv);
   return 0;
}

