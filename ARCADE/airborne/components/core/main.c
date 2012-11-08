
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

#include "util/logger/logger.h"
#include "command/command.h"
#include "util/time/ltime.h"
#include "filters/sliding_avg.h"
#include "model/model.h"
#include "control/control.h"
#include "platform/platform.h"
#include "hardware/util/calibration.h"


#define MOTORS_ENABLED 0 /* set to 0 for disabling actuators */


#define REALTIME_FREQ (200)
#define REALTIME_PERIOD (1.0 / REALTIME_FREQ)
#define CONTROL_RATIO (2)

static periodic_thread_t realtime_thread;


#if 0
void _main(int argc, char *argv[])
{
   LOG(LL_INFO, "+------------------+");
   LOG(LL_INFO, "|   core startup   |");
   LOG(LL_INFO, "+------------------+");

   LOG(LL_INFO, "initializing system");

   /* set-up real-time scheduling: */
   struct sched_param sp;
   sp.sched_priority = sched_get_priority_max(SCHED_FIFO);
   sched_setscheduler(getpid(), SCHED_FIFO, &sp);
   if (mlockall(MCL_CURRENT | MCL_FUTURE))
   {
      LOG(LL_ERROR, "mlockall() failed");
      exit(EXIT_FAILURE);
   }

   /* initialize hardware/drivers: */
   LOG(LL_INFO, "initializing platform");
   platforms_init(0);
   
   LOG(LL_INFO, "initializing model/controller");
   model_init();
   ctrl_init();
   
   /* initialize command interface */
   LOG(LL_INFO, "initializing cmd interface");
   cmd_init();


   //platform_start_motors();
   float forces[4] = {0, 0, 0, 0};
   float voltage;
   while (1)
   {
      if (platform_read_voltage(&voltage) < 0)
      {
         voltage = -1.0;   
      }
      printf("voltage: %f\n", voltage);
      sleep(1);
   }

   LOG(LL_INFO, "system up and running");
   struct timespec ts_curr;
   struct timespec ts_prev;
   struct timespec ts_diff;
   clock_gettime(CLOCK_REALTIME, &ts_curr);
   
   static float rc_bias[3];
   unsigned char i2c_buffer[4] = {0, 0, 0, 0};

   /* run model and controller: */
   while (1)
   {
      /* calculate dt: */
      ts_prev = ts_curr;
      clock_gettime(CLOCK_REALTIME, &ts_curr);
      TIMESPEC_SUB(ts_diff, ts_curr, ts_prev);
      float dt = (float)ts_diff.tv_sec + (float)ts_diff.tv_nsec / (float)NSEC_PER_SEC;

      /* read sensor values into model input structure: */
      model_input_t model_input;
      model_input.dt = dt;
      //platform_read_ahrs(&model_input.ahrs_data);
      //platform_read_gps(&model_input.gps_data);
      //platform_read_ultra(&model_input.ultra_z);
      //platform_read_baro(&model_input.baro_z);

      /* execute model step: */
      model_state_t model_state;
      model_step(&model_state, &model_input);

      EVERY_N_TIMES(100, printf("%f %f", model_state.x.pos, model_state.y.pos));
      /* execute controller step: */
      ctrl_out_t out;
      ctrl_step(&out, dt, &model_state);
 

      /* write data to motor mixer: */
      //EVERY_N_TIMES(OUTPUT_RATIO, motors_write(&out));
   }
   while (1)
   {
      sleep(1);
   }
}
#endif




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
   ctrl_init();
   
   LOG(LL_INFO, "initializing cmd/params interface");
   cmd_init();
   params_thread_start();
   
   platforms_init(0);
   printf("Press 's' and return to continue\n");
   char start_key = 0;
   while (start_key != 's')
   {
      start_key = getchar();
   }
   
   rc_data_t rc_data;
   calibration_t rc_cal;
   calibration_init(&rc_cal, 3, 400);
   unsigned int timer = 0;
   for (timer = 0; timer < 400; timer++)
   {
      platform_read_rc_raw(&rc_data);
      calibration_sample_bias(&rc_cal, channels);
      msleep(1);
   }
   rc_bias[0] = rc_bias[0]/400;
   rc_bias[1] = rc_bias[1]/400;
   rc_bias[2] = rc_bias[2]/400;
   LOG(LL_INFO, "system up and running");

   bma180_dev_t bma;
   bma180_init(&bma, &bus, BMA180_RANGE_4G, BMA180_BW_40HZ);
   bma180_avg_acc(&bma);

   hmc5883_dev_t hmc;
   hmc5883_init(&hmc, &bus);

   FILE *fp = fopen("/root/ARCADE_UAV/components/core/temp/log.dat","w");
   if (fp == NULL) printf("ERROR: could not open File");
   fprintf(fp,"gyro_x gyro_y gyro_z motor1 motor2 motor3 motor4 battery_voltage rc_input0 rc_input1 rc_input2 rc_input3 u_ctrl1 u_ctrl2 u_ctrl3\n");

   /*threadsafe_float_t speed_p;
   threadsafe_float_t speed_i;
   threadsafe_float_t speed_i_max;
   
   threadsafe_float_t pos_p;
   threadsafe_float_t pos_i;
   threadsafe_float_t pos_i_max;*/
   //ControllerData controller;
   //ControllerNew controller;
   PIIDController controller;
   Filter2 filter_out;
   filter2_lp_init(&filter_out,55.0f,0.95f,0.005f,4);

   cvxgen_init();
   piid_controller_init(&controller, REALTIME_PERIOD);
   //new_controller_init(&controller,0.003f);
   controller.f_local[0] = 1.50f;

   long timer_counter = 0;

   struct timespec ts_curr;
   struct timespec ts_prev;
   struct timespec ts_diff;
   

   bma180_read_acc(&bma);
   hmc5883_read(&hmc);

   madgwick_ahrs_t madgwick_ahrs;
   madgwick_ahrs_init(&madgwick_ahrs, 0.01);

   clock_gettime(CLOCK_REALTIME, &ts_curr);
   PERIODIC_THREAD_LOOP_BEGIN
   {
      /*
       * calculate dt:
       */
      ts_prev = ts_curr;
      clock_gettime(CLOCK_REALTIME, &ts_curr);
      TIMESPEC_SUB(ts_diff, ts_curr, ts_prev);
      float dt = (float)ts_diff.tv_sec + (float)ts_diff.tv_nsec / (float)NSEC_PER_SEC;

      /*
       * read sensor values into model input structure:
       */
      model_input_t model_input;
      model_input.dt = dt;

      float gyro_vals[3];
      itg3200_read_gyro(&itg);
      bma180_read_acc(&bma);
      hmc5883_read(&hmc);
      //platform_read_ahrs(&model_input.ahrs_data);
      //platform_read_gps(&model_input.gps_data);
      //platform_read_ultra(&model_input.ultra_z);
      //platform_read_baro(&model_input.baro_z);

  
      madgwick_ahrs_update(&madgwick_ahrs, itg.gyro.x, itg.gyro.y, itg.gyro.z, bma.raw.x, bma.raw.y, bma.raw.z, hmc.raw.x, hmc.raw.y, hmc.raw.z, 11.0, dt);
      euler_angles(madgwick_ahrs.quat.q0, madgwick_ahrs.quat.q1, madgwick_ahrs.quat.q2, madgwick_ahrs.quat.q3);
      unroll_states();
      // EVERY_N_TIMES(20, printf("%f %f %f\n", euler.x / M_PI * 180, euler.y / M_PI * 180, euler.z / M_PI * 180); fflush(stdout));
      //EVERY_N_TIMES(20, printf("%f %f %f %f\n", madgwick_ahrs.quat.q0, madgwick_ahrs.quat.q1, madgwick_ahrs.quat.q2, madgwick_ahrs.quat.q3); fflush(stdout));

      gyro_vals[0] = itg.gyro.x;
      gyro_vals[1] = -itg.gyro.y;
      gyro_vals[2] = -itg.gyro.z;

    
      //ahrs_read(&model_input.ahrs_data);
      //gps_read(&model_input.gps_data);
      //model_input.ultra_z = ultra_altimeter_read();
      //model_input.baro_z = baro_altimeter_read();
      
      mixer_in_t mixer_in;
#if 0
      /*
       * execute model step:
       */
      model_state_t model_state;
      model_step(&model_state, &model_input);

      float dir = model_input.gps_data.course; //atan2(model_state.y.speed, model_state.x.speed);
      char buf[100];
      sprintf(buf, "%f,%f;%f,%f", model_state.x.speed, model_state.y.speed, cos(dir) * model_input.gps_data.speed, sin(dir) * model_input.gps_data.speed);
      EVERY_N_TIMES(100, udp_socket_send(udp_socket, buf, strlen(buf)));

      /*
       * execute controller step:
       */
      ctrl_step(&mixer_in, dt, &model_state);
#endif
      
      /* determine attitude */
      float u_ctrl[3];
      float rc_input[4] = {0.0f, 0.0f, 0.0f, 0.0f};
      
      /* get (and scale) values from remote interface */
      float rc_pitch = (float)rc_dsl_reader_get_channel(0) / 2000.0f - rc_bias[0];
      if (fabs(rc_pitch)<0.005f) rc_pitch = 0.0f;
      float rc_roll = (float)rc_dsl_reader_get_channel(1) / 2000.0f - rc_bias[1];
      if (fabs(rc_roll)<0.005f) rc_roll = 0.0f;
      float rc_yaw = -(float)rc_dsl_reader_get_channel(3) / 2000.0f - rc_bias[2];
      if (fabs(rc_yaw)<0.005f) rc_yaw = 0.0f;
      float rc_gas = (float)rc_dsl_reader_get_channel(2) / 4000.0f + 0.5f;
      float rc_ch5 = (float)rc_dsl_reader_get_channel(4) / 2000.0f;

      /* set-up input for basic controller: */
      rc_input[0] = -2.0f * rc_roll + 0.0 * mixer_in.roll; // 0.1
      rc_input[1] = 2.0f * rc_pitch - 0.0 * mixer_in.pitch; // 0.1
      rc_input[2] = -3.0f * rc_yaw + 0.0 * mixer_in.yaw;
      controller.f_local[0] = 30.0f * rc_gas;
      
      piid_controller_run(&controller,gyro_vals,rc_input,u_ctrl);
      filter2_run(&filter_out,controller.f_local,controller.f_local); // TODO: WAR VORHER DRIN!!
      controller.int_enable = force2twi(controller.f_local, &voltage,i2c_buffer);
      //fprintf(fp,"%10.9f %10.9f %10.9f %d %d %d %d %6.4f %6.4f %6.4f %6.4f %6.4f %10.9f %10.9f %10.9f\n",gyro_vals[0],gyro_vals[1],gyro_vals[2],i2c_buffer[0],i2c_buffer[1],i2c_buffer[2],i2c_buffer[3],voltage,controller.f_local[0],rc_input[0], rc_input[1], rc_input[2], u_ctrl[0], u_ctrl[1], u_ctrl[2]);
      //mixer_in_t mixer_in;
      if (1) //rc_ch5 > 0.0 || !rc_dsl_reader_signal_valid())
      {
         //flag = 0;
         /*mixer_in.pitch = -pitch_ctrl_val;
         mixer_in.roll = -roll_ctrl_val;
         mixer_in.yaw = -yaw_ctrl_val;
         mixer_in.gas = gas_sp;*/
         controller.int_enable = 0;
         i2c_buffer[0] = 0;
         i2c_buffer[1] = 0;
         i2c_buffer[2] = 0;
         i2c_buffer[3] = 0;
      }

      EVERY_N_TIMES(2, motors_write_uint8(i2c_buffer));
   }
   PERIODIC_THREAD_LOOP_END
#endif
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


int _main(int argc, char *argv[])
{
   (void)argc;
   (void)argv;
   const struct timespec realtime_period = {0, REALTIME_PERIOD * NSEC_PER_MSEC};
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


