
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

#include "util.h"
#include "util/logger/logger.h"
#include "util/opcd_params/opcd_params.h"
#include "interfaces/cmd.h"
#include "sensor_actor/lib/i2c/omap_i2c_bus.h"
#include "sensor_actor/flight_gear/fg_reader.h"
#include "sensor_actor/interfaces/altimeter.h"
#include "sensor_actor/leds/leds_overo.h"
#include "sensor_actor/interfaces/ahrs.h"
#include "sensor_actor/interfaces/gps.h"
#include "sensor_actor/interfaces/motors.h"
#include "../../../../common/scl/src/sclhelper.h"
#include "util/threads/threadsafe_types.h"
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
   
   syslog(LOG_INFO, "initializing signaling and communication link (SCL)");
   if (scl_init("core") != 0)
   {
      syslog(LOG_CRIT, "could not init scl module");
      exit(EXIT_FAILURE);
   }
   
   syslog(LOG_INFO, "initializing opcd interface");
   opcd_params_init("core.");
   
   syslog(LOG_INFO, "opening logger");
   if (logger_open() != 0)
   {
      syslog(LOG_CRIT, "could not open logger");
      exit(EXIT_FAILURE);
   }


   sleep(1);

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

   omap_i2c_bus_init();
   
   baro_altimeter_init();
   ultra_altimeter_init();
   ahrs_init();
   leds_overo_initialize();
   gps_init();
   
   LOG(LL_INFO, "initializing model/controller");
   model_init();
   ctrl_init();
   
   LOG(LL_INFO, "initializing cmd/params interface");
   cmd_init();
   
   for(int n = 0; n < NUM_AVG; n++)
   {
      output_avg[n] = sliding_avg_create(OUTPUT_RATIO, 0.0f);
   }
   LOG(LL_INFO, "system up and running");
  
   struct timespec ts_curr;
   struct timespec ts_prev;
   struct timespec ts_diff;
   clock_gettime(CLOCK_REALTIME, &ts_curr);

   while(1)
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
      ahrs_read(&model_input.ahrs_data);
      gps_read(&model_input.gps_data);
      model_input.ultra_z = ultra_altimeter_read();
      model_input.baro_z = baro_altimeter_read();

      /*
       * execute model step:
       */
      model_state_t model_state;
      model_step(&model_state, &model_input);

      /*
       * execute controller step:
       */
      mixer_in_t mixer_in;
      ctrl_step(&mixer_in, dt, &model_state);

      /*
       * write data to mixer at OUTPUT_RATIO:
       */
      mixer_in.pitch = sliding_avg_calc(output_avg[AVG_PITCH], mixer_in.pitch);
      mixer_in.roll = sliding_avg_calc(output_avg[AVG_ROLL], mixer_in.roll);
      mixer_in.yaw = sliding_avg_calc(output_avg[AVG_YAW], mixer_in.yaw);
      mixer_in.gas = sliding_avg_calc(output_avg[AVG_GAS], mixer_in.gas);
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

