
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
#include "scl_cmd/scl_cmd.h"
#include "sensor_actor/lib/i2c/omap_i2c_bus.h"
#include "sensor_actor/interfaces/altimeter.h"
#include "sensor_actor/interfaces/motors.h"
#include "sensor_actor/interfaces/ahrs.h"
#include "sensor_actor/interfaces/gps.h"
#include "sensor_actor/interfaces/motors.h"
#include "sensor_actor/voltage/voltage_reader.h"
#include "util/time/ltime.h"
#include "algorithms/sliding_avg.h"
#include "model/model.h"
#include "controllers/ctrl.h"


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
   omap_i2c_bus_init();
   baro_altimeter_init();
   ultra_altimeter_init();
   ahrs_init();
   motors_init();
   voltage_reader_start();
   //gps_init();
   
   LOG(LL_INFO, "initializing model/controller");
   float dt = 0.03;
   model_init(dt);
   ctrl_init();
   
   /* initialize command interface */
   LOG(LL_INFO, "initializing cmd interface");
   cmd_init();
   
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
      ahrs_read(&model_input.ahrs_data);
      gps_read(&model_input.gps_data);
      model_input.ultra_z = ultra_altimeter_read();
      model_input.baro_z = baro_altimeter_read();

      /* execute model step: */
      model_state_t model_state;
      model_step(&model_state, &model_input);

      /* execute controller step: */
      mixer_in_t mixer_in;
      ctrl_step(&mixer_in, dt, &model_state);
 
      /* set up mixer input: */
      mixer_in.pitch = sliding_avg_calc(output_avg[AVG_PITCH], mixer_in.pitch);
      mixer_in.roll = sliding_avg_calc(output_avg[AVG_ROLL], mixer_in.roll);
      mixer_in.yaw = sliding_avg_calc(output_avg[AVG_YAW], mixer_in.yaw);
      mixer_in.gas = sliding_avg_calc(output_avg[AVG_GAS], mixer_in.gas);

      /* write data to motor mixer: */
      EVERY_N_TIMES(OUTPUT_RATIO, motors_write(&mixer_in));
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
   //_main(argc, argv);
   daemonize("/var/run/core.pid", _main, _cleanup, argc, argv);
   return 0;
}

