
#include <syslog.h>
#include <unistd.h>

#include <opcd_params.h>
#include <util.h>
#include <sclhelper.h>
#include <msgpack.h>

#include "main_loop.h"
#include "main_util.h"
#include "../util/time/interval.h"
#include "../util/logger/logger.h"
#include "../command/command.h"
#include "../estimators/ahrs.h"
#include "../estimators/pos.h"
#include "../control/control.h"
#include "../platform/platform.h"
#include "../platform/arcade_quadro.h"
#include "../control/basic/piid.h"
#include "../control/basic/feed_forward.h"
#include "../hardware/util/calibration.h"
#include "../hardware/util/gps_util.h"
#include "../hardware/util/mag_decl.h"
#include "../control/position/navi.h"
#include "../control/position/att_ctrl.h"
#include "../control/position/z_ctrl.h"
#include "../control/position/yaw_ctrl.h"
#include "../control/speed/xy_speed.h"
#include "../state/motors_state.h"
#include "../calpub.h"
#include "../filters/sliding_var.h"
#include "../behaviors/landing.h"


static int calibrate = 0;
static int motors_locked = 0;
static void *debug_socket = NULL;
static msgpack_sbuffer *msgpack_buf;
static msgpack_packer *pk;
static float mag_bias = 0.0f;
static float mag_decl = 0.0f;
static gps_data_t gps_data;
static gps_rel_data_t gps_rel_data = {0.0, 0.0, 0.0};
static Filter1 rc_valid_filter;
static calibration_t gyro_cal;
static calibration_t rc_cal;
static feed_forward_t feed_forward;
static piid_t piid;
static ahrs_t ahrs;
static quat_t start_quat;
static gps_util_t gps_util;
static interval_t gyro_move_interval;
static char *dbg_spec[] = {"dt",                           /*  1 */
   "gyro_x", "gyro_y", "gyro_z",                           /*  2 -  4 */
   "acc_x", "acc_y", "acc_z",                              /*  5 -  7 */
   "mag_x", "mag_y", "mag_z",                              /*  8 - 10 */
   "q0", "q1", "q2", "q3",                                 /* 11 - 14 */
   "yaw", "pitch", "roll",                                 /* 15 - 17 */
   "acc_e", "acc_n", "acc_u",                              /* 18 - 20 */
   "raw_e", "raw_n", "raw_ultra_u", "raw_baro_u",          /* 21 - 24 */
   "pos_e", "pos_n", "pos_ultra_u", "pos_baro_u",          /* 25 - 28 */
   "speed_e",  "pos_n", "speed_ultra_u", "pos_ultra_u",    /* 29 - 32 */
   "yaw_sp", "pitch_sp", "roll_sp",                        /* 33 - 35 */
   "flight_state", "rc_valid",                             /* 36 - 37 */
   "rc_pitch", "rc_roll", "rc_yaw", "rc_gas", "rc_switch", /* 38 - 42 */
   "sensor_status"};


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
manual_mode = M_ATT_REL;



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



void main_init(int override_hw)
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


   if (!override_hw)
   {
      LOG(LL_INFO, "initializing platform");
      if (platform_init(arcade_quadro_init) < 0)
      {
         LOG(LL_ERROR, "could not initialize platform");
         die();
      }
      calpub_init();
   }

   LOG(LL_INFO, "initializing model/controller");
   pos_init();
   xy_speed_ctrl_init();
   att_ctrl_init();
   yaw_ctrl_init();
   z_ctrl_init(platform_param()->mass_kg * 10.0f / platform_param()->max_thrust_n);
   navi_init();

   LOG(LL_INFO, "initializing command interface");
   cmd_init();

   motors_state_init(0.12f, 0.8f);
   debug_socket = scl_get_socket("debug");

   /* send debug header: */
   msgpack_buf = msgpack_sbuffer_new();
   pk = msgpack_packer_new(msgpack_buf, msgpack_sbuffer_write);
   msgpack_pack_array(pk, ARRAY_SIZE(dbg_spec));
   FOR_EACH(i, dbg_spec)
   {
      size_t len = strlen(dbg_spec[i]);
      msgpack_pack_raw(pk, len);
      msgpack_pack_raw_body(pk, dbg_spec[i], len);
   }
   scl_copy_send_dynamic(debug_socket, msgpack_buf->data, msgpack_buf->size);

   /* init calibration data: */
   cal_init(&gyro_cal, 3, 500);
   cal_init(&rc_cal, 3, 500);
   
   /* init stabilizing controller: */
   feed_forward_init(&feed_forward, REALTIME_PERIOD);
   piid_init(&piid, REALTIME_PERIOD);

   ahrs_init(&ahrs, 10.0f, 2.0f * REALTIME_PERIOD, 0.02f);
   gps_util_init(&gps_util);
   flight_state_init(50, 20, 2.0, 150.0, 1.3);
   
   interval_init(&gyro_move_interval);

   gps_data_init(&gps_data);
   
   filter1_lp_init(&rc_valid_filter, 0.5, REALTIME_PERIOD, 1);

   LOG(LL_INFO, "entering main loop");
}


void main_step(float dt, marg_data_t *marg_data, gps_data_t *gps_data, float ultra, float baro, float voltage, float channels[MAX_CHANNELS], uint16_t sensor_status, int override_hw)
{
   /*******************************************
    * read sensor data and calibrate sensors: *
    *******************************************/
   pos_in_t pos_in;
   pos_in.dt = dt;
   pos_in.ultra_z = ultra;
   pos_in.baro_z = baro;
   
   if (!(sensor_status & MARG_VALID))
      goto out;
   
   float rc_valid_f = (sensor_status & RC_VALID) ? 1.0f : 0.0f;
   filter1_run(&rc_valid_filter, &rc_valid_f, &rc_valid_f);
   int rc_valid = rc_valid_f > 0.5f;
   if (!rc_valid)
   {
      memset(channels, 0, sizeof(channels));   
   }
   else
   {
      float cal_channels[3] = {channels[CH_PITCH], channels[CH_ROLL], channels[CH_YAW]};
      cal_sample_apply(&rc_cal, cal_channels);
      channels[CH_PITCH] = cal_channels[0];
      channels[CH_ROLL] = cal_channels[1];
      channels[CH_YAW] = cal_channels[2];
   }

   if (cal_sample_apply(&gyro_cal, &marg_data->gyro.vec[0]) == 0 && gyro_moved(&marg_data->gyro))
   {
      if (interval_measure(&gyro_move_interval) > 1.0)
         LOG(LL_ERROR, "gyro moved during calibration, retrying");
      cal_reset(&gyro_cal);
   }

   if (calibrate)
   {
      EVERY_N_TIMES(10, calpub_send(marg_data));
      return;
   }
   
   if (sensor_status & GPS_VALID)
   {
      gps_util_update(&gps_rel_data, &gps_util, gps_data);
      pos_in.dx = gps_rel_data.dx;
      pos_in.dy = gps_rel_data.dy;
      ONCE(LOG(LL_ERROR, "declination lookup yields: %f", mag_decl);
           mag_decl = mag_decl_lookup(gps_data->lat, gps_data->lon));
   }

   /* calibration: */
   acc_mag_apply_cal(&marg_data->acc, &marg_data->mag);


   /********************************
    * perform sensor data fusion : *
    ********************************/

   int ahrs_state = ahrs_update(&ahrs, marg_data, dt);
   flight_state_t flight_state = flight_state_update(&marg_data->acc.vec[0], pos_in.ultra_z);
   
   /* global z orientation calibration: */
   quat_t zrot_quat = {{mag_decl + mag_bias, 0, 0, -1}};
   quat_mul(&ahrs.quat, &zrot_quat, &ahrs.quat);
   
   /* compute euler angles from quaternion: */
   euler_t euler;
   quat_to_euler(&euler, &ahrs.quat);

   if (ahrs_state < 0 || !cal_complete(&gyro_cal))
      goto out;

   ONCE(start_quat = ahrs.quat; LOG(LL_DEBUG, "system initialized; orientation = yaw: %f pitch: %f roll: %f", euler.yaw, euler.pitch, euler.roll));
   
   /* compute NEU accelerations using quaternion: */
   quat_rot_vec(&pos_in.acc, &marg_data->acc, &ahrs.quat);
   pos_in.acc.z *= -1.0f; /* <-- transform from NED to NEU frame */
   
   /* compute next 3d position estimate: */
   pos_t pos_estimate;
   pos_update(&pos_estimate, &pos_in);


   /*******************************
    * run high-level controllers: *
    *******************************/

   ctrl_out_t auto_stick;
   float yaw_err, z_err;
   auto_stick.yaw = yaw_ctrl_step(&yaw_err, euler.yaw, marg_data->gyro.z, dt);
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
   vec2_t pitch_roll_speed = {{marg_data->gyro.y, marg_data->gyro.x}};
   vec2_t pitch_roll_ctrl;
   att_ctrl_step(&pitch_roll_ctrl, &att_err, dt, &pitch_roll, &pitch_roll_speed, &pitch_roll_sp);
   auto_stick.pitch = pitch_roll_ctrl.x;
   auto_stick.roll = pitch_roll_ctrl.y;

   /************************************
    * combine auto stick and RC stick: *
    ************************************/

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

   if (rc_valid && channels[CH_SWITCH] > 0.5 && mode != CM_FULL_AUTO)
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

   float gyro_vals[3] = {marg_data->gyro.x, -marg_data->gyro.y, -marg_data->gyro.z};
   feed_forward_run(&feed_forward, &f_local.vec[1], piid_sp);
   piid_run(&piid, &f_local.vec[1], gyro_vals, piid_sp);


   /********************
    * actuator output: *
    ********************/

   /* requirements specification for take-off: */
   int common_require = sensor_status & (VOLTAGE_VALID | ULTRA_VALID);
   int manual_require = common_require && (manual_mode == M_ATT_GPS_SPEED ? (sensor_status & GPS_VALID) : 1) && rc_valid && channels[CH_SWITCH] > 0.5;
   int full_auto_require = common_require && (sensor_status & (BARO_VALID | GPS_VALID));
   int safe_auto_require = full_auto_require && rc_valid && channels[CH_SWITCH] > 0.5;
   
   int satisfied = 0; /* initial value applies to CM_DISABLED */
   if (mode == CM_MANUAL)
      satisfied = manual_require;
   else if (mode == CM_FULL_AUTO)
      satisfied = full_auto_require;
   else if (mode == CM_SAFE_AUTO)
      satisfied = safe_auto_require;

   /* start motors if requirements are met AND conditions apply;
    * stopping the motors does not depend on requirements: */
   motors_state_update(flight_state, motors_locked, norm_gas, dt, satisfied);
   if (!motors_state_controllable())
   {
      memset(&f_local, 0, sizeof(f_local)); /* all moments are 0 / minimum motor RPM */
      piid_reset(&piid); /* reset piid integrators so that we can move the device manually */
      /* TODO: also reset higher-level controllers */
   }

   int motors_enabled = 0;
   if (rc_valid)
   {
      motors_enabled = mode == CM_MANUAL && channels[CH_SWITCH] > 0.5;
   }
   
   /* emergency landing: */
   if (!rc_valid || landing_started())
   {
      motors_enabled = landing_run(&f_local.gas, pos_estimate.ultra_z.pos, pos_estimate.baro_z.pos, dt);
   }
   
   /* write forces to motors: */
   if (motors_state_safe())
   {
      motors_enabled = 0;
   }

   if (!override_hw)
   {
      int mot_status = platform_write_motors(motors_enabled, f_local.vec, voltage);
      piid.int_enable = mot_status & MOTORS_INT_ENABLE ? 1 : 0;
   }

out:
   /* publish debug data: */
   msgpack_sbuffer_clear(msgpack_buf);
   msgpack_pack_array(pk, ARRAY_SIZE(dbg_spec));
   #define PACKI(val) msgpack_pack_int(pk, val) /* pack integer */
   #define PACKF(val) msgpack_pack_float(pk, val) /* pack float */
   #define PACKFV(ptr, n) FOR_N(i, n) PACKF(ptr[i]) /* pack float vector */
   PACKF(dt);
   PACKFV(marg_data->gyro.vec, 3);
   PACKFV(marg_data->acc.vec, 3);
   PACKFV(marg_data->mag.vec, 3);
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
   PACKI(rc_valid);
   PACKFV(channels, 5);
   PACKI(sensor_status);
   scl_copy_send_dynamic(debug_socket, msgpack_buf->data, msgpack_buf->size);
}

