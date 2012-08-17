
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
#include <sclhelper.h>

#include "util/logger/logger.h"
#include "command/command.h"
#include "util/time/ltime.h"
#include "filters/sliding_avg.h"
#include "model/model.h"
#include "control/control.h"
#include "platform/platform.h"


#define OUTPUT_RATIO 3 /* 300Hz/OUTPUT_RATIO = output rate (averaged) */


enum
{
   AVG_PITCH = 0,
   AVG_ROLL,
   AVG_YAW,
   AVG_GAS,
   NUM_AVG
};


static sliding_avg_t *output_avg[NUM_AVG];


void _main(int argc, char *argv[])
{
   (void)argc;
   (void)argv;
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
      voltage = 15.0;   
   }
   printf("voltage: %f\n", voltage);
   platform_write_motors(forces, voltage, NULL);
   sleep(1);
}
   /* prepare main loop: */
   for (int i = 0; i < NUM_AVG; i++)
   {
      output_avg[i] = sliding_avg_create(OUTPUT_RATIO, 0.0f);
   }

   LOG(LL_INFO, "system up and running");
   struct timespec ts_curr;
   struct timespec ts_prev;
   struct timespec ts_diff;
   clock_gettime(CLOCK_REALTIME, &ts_curr);
 
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
      platform_read_ahrs(&model_input.ahrs_data);
      platform_read_gps(&model_input.gps_data);
      platform_read_ultra(&model_input.ultra_z);
      platform_read_baro(&model_input.baro_z);

      /* execute model step: */
      model_state_t model_state;
      model_step(&model_state, &model_input);

      /* execute controller step: */
      ctrl_out_t out;
      ctrl_step(&out, dt, &model_state);
 
      /* set up mixer input: */
      out.pitch = sliding_avg_calc(output_avg[AVG_PITCH], out.pitch);
      out.roll = sliding_avg_calc(output_avg[AVG_ROLL], out.roll);
      out.yaw = sliding_avg_calc(output_avg[AVG_YAW], out.yaw);
      out.gas = sliding_avg_calc(output_avg[AVG_GAS], out.gas);

      /* write data to motor mixer: */
      //EVERY_N_TIMES(OUTPUT_RATIO, motors_write(&out));
   }
}


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


int main(int argc, char *argv[])
{
   _main(argc, argv);
   daemonize("/var/run/core.pid", _main, _cleanup, argc, argv);
   return 0;
}

