
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
#include <msgpack.h>

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


enum
{
   M_ATT_REL, /* relative attitude control (aka heading hold, axis lock,...) */
   M_ATT_ABS, /* absolute attitude control (aka acc-mode) */
   M_ATT_GPS_SPEED /* stick defines GPS speed for local coordinate frame */
}
manual_mode = M_ATT_ABS;


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


static void realtime_init(void)
{
   ASSERT_ONCE();

   LOG(LL_INFO, "setting maximum CPU clock");
   if (system("echo performance > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor") != 0)
   {
      LOG(LL_ERROR, "failed");
      die();
   }
 
   LOG(LL_INFO, "setting up real-time scheduling");
   static struct sched_param sp;
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

   realtime_init();

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

   motostate_init(1.0f, 0.15f, 0.1f);

   void *debug_socket = scl_get_socket("debug");

   /* initialize msgpack helpers: */
   msgpack_sbuffer *msgpack_buf = msgpack_sbuffer_new();
   msgpack_packer *pk = msgpack_packer_new(msgpack_buf, msgpack_sbuffer_write);
   char *dbg_spec[] = {"dt",                                   /*  1 */
      "gyro_x", "gyro_y", "gyro_z",                            /*  2 -  4 */
      "acc_x", "acc_y", "acc_z",                               /*  5 -  7 */
      "mag_x", "mag_y", "mag_z",                               /*  8 - 10 */
      "q0", "q1", "q2", "q3",                                  /* 11 - 14 */
      "yaw", "pitch", "roll",                                  /* 15 - 17 */
      "acc_e", "acc_n", "acc_u",                               /* 18 - 20 */
      "raw_e", "raw_n", "raw_ultra_u", "raw_baro_u",           /* 21 - 24 */
      "pos_e", "pos_n", "pos_ultra_u", "pos_baro_u",           /* 25 - 28 */
      "speed_e",  "pos_n", "speed_ultra_u", "pos_ultra_u",     /* 29 - 32 */
      "yaw_sp", "pitch_sp", "roll_sp",                         /* 33 - 35 */
      "flight_state", "rc_valid",                              /* 36 - 37 */
      "rc_pitch", "rc_roll", "rc_yaw", "rc_gas", "rc_switch"}; /* 38 - 42 */
   /* send header: */
   msgpack_pack_array(pk, ARRAY_SIZE(dbg_spec));
   FOR_EACH(i, dbg_spec)
   {
      size_t len = strlen(dbg_spec[i]);
      msgpack_pack_raw(pk, len);
      msgpack_pack_raw_body(pk, dbg_spec[i], len);
   }
   scl_copy_send_dynamic(debug_socket, msgpack_buf->data, msgpack_buf->size);

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
   gps_rel_data_t gps_rel_data = {0.0, 0.0, 0.0};

   flight_detect_init(50, 20, 2.0, 150.0);
   
   LOG(LL_INFO, "entering main loop");
   interval_t interval, gyro_move_interval;
   interval_init(&interval);
   interval_init(&gyro_move_interval);

   periodic_thread_t _thread; periodic_thread_t *thread = &_thread;
   _thread.name = "mainloop"; _thread.running = 1;
   _thread.periodic_data.period.tv_sec = 0;
   _thread.periodic_data.period.tv_nsec = NSEC_PER_SEC * REALTIME_PERIOD;
   float mag_bias = 0.0f;
   float mag_decl = 0.0f;
   gps_data_t gps_data;
   memset(&gps_data, 0, sizeof(gps_data));
   PERIODIC_THREAD_LOOP_BEGIN
   {
      
      /*******************************************
       * read sensor data and calibrate sensors: *
       *******************************************/
      float dt = interval_measure(&interval);
      pos_in_t pos_in;
      pos_in.dt = dt;
      
      float voltage = 16.0f;
      float channels[MAX_CHANNELS];
      marg_data_t marg_data;
      uint16_t sensor_status = platform_read_sensors(&marg_data, &gps_data, &pos_in.ultra_z, &pos_in.baro_z, &voltage, channels);
      if (!(sensor_status & MARG_VALID))
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
      
      if (sensor_status & GPS_VALID)
      {
         gps_util_update(&gps_rel_data, &gps_util, &gps_data);
         pos_in.dx = gps_rel_data.dx;
         pos_in.dy = gps_rel_data.dy;
         ONCE(LOG(LL_ERROR, "declination lookup yields: %f", mag_decl);
              mag_decl = mag_decl_lookup(gps_data.lat, gps_data.lon));
      }

      /* calibration: */
      acc_mag_apply_cal(&marg_data.acc, &marg_data.mag);

 
      /********************************
       * perform sensor data fusion : *
       ********************************/

      int ahrs_state = ahrs_update(&ahrs, &marg_data, dt);
      flight_state_t flight_state = flight_detect(&marg_data.acc.vec[0]);
      
      /* global z orientation calibration: */
      quat_t zrot_quat = {{mag_decl + mag_bias, 0, 0, -1}};
      quat_mul(&ahrs.quat, &zrot_quat, &ahrs.quat);
      
      /* compute euler angles from quaternion: */
      euler_t euler;
      quat_to_euler(&euler, &ahrs.quat);

      if (ahrs_state == 1)
      {
         start_quat = ahrs.quat;
         LOG(LL_DEBUG, "initial orientation - yaw: %f pitch: %f roll: %f", euler.yaw, euler.pitch, euler.roll);
      }
      if (ahrs_state < 0 || !cal_complete(&gyro_cal))
         continue;

      ONCE(LOG(LL_INFO, "system initialized"));
      
      /* compute NEU accelerations using quaternion: */
      quat_rot_vec(&pos_in.acc, &marg_data.acc, &ahrs.quat);
      pos_in.acc.z *= -1.0f; /* <-- transform from NED to NEU frame */
      
      /* compute next 3d position estimate: */
      pos_t pos_estimate;
      pos_update(&pos_estimate, &pos_in);


      /*******************************
       * run high-level controllers: *
       *******************************/

      ctrl_out_t auto_stick;
      float yaw_err, z_err;
      auto_stick.yaw = yaw_ctrl_step(&yaw_err, euler.yaw, marg_data.gyro.z, dt);
      auto_stick.gas = z_ctrl_step(&z_err, pos_estimate.ultra_z.pos,
                                   pos_estimate.baro_z.pos, pos_estimate.baro_z.speed, dt);
      
      vec2_t speed_sp;
      if (mode == CM_MANUAL && manual_mode == M_ATT_GPS_SPEED)
      {
         /* direct speed control via stick: */
         speed_sp.x = 10.0f * channels[CH_PITCH] * cosf(euler.yaw);
         speed_sp.y = 10.0f * channels[CH_ROLL] * sinf(euler.yaw);
      }
      else
      {
         /* run xy navigation controller: */
         vec2_t pos_vec = {{pos_estimate.x.pos, pos_estimate.y.pos}};
         navi_run(&speed_sp, &pos_vec, dt);
      }

      /* run speed vector controller: */
      vec2_t pitch_roll_sp = {{0.0f, 0.0f}};
      vec2_t speed_vec = {{pos_estimate.x.speed, pos_estimate.y.speed}};
      xy_speed_ctrl_run(&pitch_roll_sp, &speed_sp, &speed_vec, euler.yaw);
      FOR_N(i, 2) pitch_roll_sp.vec[i] = sym_limit(pitch_roll_sp.vec[i], 0.2);

      /* run attitude controller: */
      vec2_t pitch_roll = {{euler.pitch, euler.roll}};
      if (manual_mode == M_ATT_ABS)
      {
         /* interpret sticks as pitch and roll setpoints: */
         pitch_roll_sp.x = -1.0f * channels[CH_PITCH];
         pitch_roll_sp.y = 1.0f * channels[CH_ROLL];
      }
      vec2_t att_err;
      vec2_t pitch_roll_speed = {{marg_data.gyro.y, marg_data.gyro.x}};
      vec2_t pitch_roll_ctrl;
      att_ctrl_step(&pitch_roll_ctrl, &att_err, dt, &pitch_roll, &pitch_roll_speed, &pitch_roll_sp);
      auto_stick.pitch = pitch_roll_ctrl.x;
      auto_stick.roll = pitch_roll_ctrl.y;

      /*************************************
       * run basic stabilizing controller: *
       *************************************/

      float piid_sp[3] = {0.0f, 0.0f, 0.0f};
      f_local_t f_local = {{0.0f, 0.0f, 0.0f, 0.0f}};
      float norm_gas = 0.0f; /* interval: [0.0 ... 1.0] */

      if (mode >= CM_SAFE_AUTO || (mode == CM_MANUAL && manual_mode == M_ATT_ABS))
      {
         piid_sp[PIID_PITCH] = -auto_stick.pitch;
         piid_sp[PIID_ROLL] = auto_stick.roll;
      }

      if (mode >= CM_SAFE_AUTO)
      {
         piid_sp[PIID_YAW] = auto_stick.yaw;
         norm_gas = auto_stick.gas;
      }

      if ((sensor_status & RC_VALID) && channels[CH_SWITCH] > 0.5 && mode != CM_FULL_AUTO)
      {
         /* mix in rc signals: */
         if (manual_mode == M_ATT_REL)
         {
            piid_sp[PIID_PITCH] += RC_PITCH_ROLL_STICK_P * channels[CH_PITCH];
            piid_sp[PIID_ROLL] += RC_PITCH_ROLL_STICK_P * channels[CH_ROLL];
         }
         piid_sp[PIID_YAW] -= RC_YAW_STICK_P * channels[CH_YAW];

         /* limit thrust using "gas" stick: */
         float stick_gas = channels[CH_GAS];
         if (   (mode == CM_SAFE_AUTO && stick_gas < norm_gas)
             || (mode == CM_MANUAL))
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
      int common_require = sensor_status & (VOLTAGE_VALID | ULTRA_VALID);
      int manual_require = common_require && (manual_mode == M_ATT_GPS_SPEED ? (sensor_status & GPS_VALID) : 1) && (sensor_status & RC_VALID) && channels[CH_SWITCH] > 0.5;
      int full_auto_require = common_require && (sensor_status & (BARO_VALID | GPS_VALID));
      int safe_auto_require = full_auto_require && (sensor_status & RC_VALID) && channels[CH_SWITCH] > 0.5;
      
      int satisfied = 0; /* initial value applies to CM_DISABLED */
      if (mode == CM_MANUAL)
         satisfied = manual_require;
      else if (mode == CM_FULL_AUTO)
         satisfied = full_auto_require;
      else if (mode == CM_SAFE_AUTO)
         satisfied = safe_auto_require;

      /* start motors if requirements are met AND conditions apply;
       * stopping the motors does not depend on requirements: */
      motostate_update(pos_in.ultra_z, flight_state, norm_gas, dt, satisfied);
      if (!motostate_controllable())
      {
         memset(&f_local, 0, sizeof(f_local)); /* all moments are 0 / minimum motor RPM */
         piid_reset(&piid); /* reset piid integrators so that we can move the device manually */
         /* TODO: also reset higher-level controllers */
      }

      /* write forces to motors: */
      int mot_status = platform_write_motors(/*motostate_enabled()*/0, f_local.vec, voltage);
      piid.int_enable = mot_status & MOTORS_INT_ENABLE ? 1 : 0;
 
      /* publish debug data: */
      msgpack_sbuffer_clear(msgpack_buf);
      msgpack_pack_array(pk, ARRAY_SIZE(dbg_spec));
      #define PACKI(val) msgpack_pack_int(pk, val) /* pack integer */
      #define PACKF(val) msgpack_pack_float(pk, val) /* pack float */
      #define PACKFV(ptr, n) FOR_N(i, n) PACKF(ptr[i]) /* pack float vector */
      PACKF(dt);
      PACKFV(marg_data.gyro.vec, 3);
      PACKFV(marg_data.acc.vec, 3);
      PACKFV(marg_data.mag.vec, 3);
      PACKFV(ahrs.quat.vec, 4);
      PACKFV(euler.vec, 3);
      PACKFV(pos_in.acc.vec, 3);
      PACKF(pos_in.dx); PACKF(pos_in.dy);
      PACKF(pos_in.ultra_z); PACKF(pos_in.baro_z);
      PACKF(pos_estimate.x.pos); PACKF(pos_estimate.y.pos);
      PACKF(pos_estimate.ultra_z.pos); PACKF(pos_estimate.baro_z.pos);
      PACKF(pos_estimate.x.speed); PACKF(pos_estimate.y.speed);
      PACKF(pos_estimate.ultra_z.speed); PACKF(pos_estimate.baro_z.speed);
      PACKF(0.0f);
      PACKFV(pitch_roll_sp.vec, 2);
      PACKI(flight_state);
      PACKI(sensor_status & RC_VALID ? 1 : 0);
      PACKFV(channels, 5);
      scl_copy_send_dynamic(debug_socket, msgpack_buf->data, msgpack_buf->size);
   }
   PERIODIC_THREAD_LOOP_END
}


int main(int argc, char *argv[])
{
   _main(argc, argv);
   daemonize("/var/run/core.pid", _main, die, argc, argv);
   return 0;
}

