
/*
 * file: holger_blmc.c
 * purpose: mikrokopter brushless motor controller interface
 * author: Tobias Simon, Ilmenau University of Technology
 */


#include <malloc.h>
#include <string.h>
#include <util.h>
#include <periodic_thread.h>
#include <unistd.h>

#include "holger_blmc_driver.h"
#include "../../libs/holger_blmc/holger_blmc.h"
#include "../../libs/holger_blmc/force2twi.h"
#include "../../../util/logger/logger.h"


static bool started = false;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static unsigned int n_motors;
static uint8_t *i2c_setp = NULL;
static uint8_t *rpm = NULL;

static periodic_thread_t motors_thread;
#define RT_MOTORS_PERIOD (6)


PERIODIC_THREAD_BEGIN(motors_thread_func)
{
   PERIODIC_THREAD_LOOP_BEGIN
   {
      holger_blmc_write(i2c_setp);
   }
   PERIODIC_THREAD_LOOP_END
}
PERIODIC_THREAD_END



void holger_blmc_driver_init(i2c_bus_t *bus, uint8_t *addrs, coupling_t *coupling, unsigned int _n_motors)
{
   ASSERT_ONCE();
   n_motors = _n_motors;
   force2twi_init(coupling);
   holger_blmc_init(bus, addrs, n_motors);
   i2c_setp = malloc(n_motors);
   memset(i2c_setp, HOLGER_I2C_OFF, n_motors);
   rpm =  malloc(n_motors);
   memset(rpm, 0, n_motors);
   struct timespec motors_period = {0, RT_MOTORS_PERIOD * NSEC_PER_MSEC};
   periodic_thread_start(&motors_thread, motors_thread_func, "motors_thread",
                         99, motors_period, NULL);
}



static void i2c_start_motors(void)
{
   pthread_mutex_lock(&mutex);
   memset(i2c_setp, HOLGER_I2C_MIN, n_motors);
   pthread_mutex_unlock(&mutex);
}


static void i2c_stop_motors(void)
{
   pthread_mutex_lock(&mutex);
   memset(i2c_setp, HOLGER_I2C_OFF, n_motors);
   pthread_mutex_lock(&mutex);
}


static void spin_down(void)
{
   /* switch off motors command: */
   i2c_stop_motors();

   /* wait until all motors are stopped: */
again:
   msleep(100);
   pthread_mutex_lock(&mutex);
   for (int i = 0; i < n_motors; i++)
   {
      if (rpm[i] != 0)
      {
         pthread_mutex_unlock(&mutex);
         goto again;
      }
   }
   pthread_mutex_unlock(&mutex);

   LOG(LL_ERROR, "motors are stopped");
}


static int spin_up(void)
{
   int retry_count = 0;
retry:
   LOG(LL_DEBUG, "starting motors");
   i2c_start_motors();
   sleep(1);
   int valid_count = 0;
   int fail_timer = 1000;

   while (1)
   {
      msleep(10);
      valid_count++;
      
      if (fail_timer-- == 0)
      {
          LOG(LL_ERROR, "starting failed, restarting");
          spin_down();
          if (++retry_count == 3)
          {
             return -1;
          }
          goto retry;
      }
      
      /* reset valid_count, if motor rpm too low */
      pthread_mutex_lock(&mutex);
      for (int i = 0; i < n_motors; i++)
      {
         if (rpm[i] < 1000.0f || rpm[i] > 50000.0f)
         {
            valid_count = 0;
            break;
         }
      }
      pthread_mutex_unlock(&mutex);
      
      if (valid_count > 200)
      {
         LOG(LL_ERROR, "motors are running");
         return 0;
      }
   }
}


int holger_blmc_driver_start_motors(void)
{
   printf("holger_blmc_driver_start_motors\n");
   pthread_mutex_lock(&mutex);
   if (!started)
   {
      memset(i2c_setp, HOLGER_I2C_MIN, n_motors);
      sleep(2);   
      started = true;
   }
   pthread_mutex_unlock(&mutex);
}


void holger_blmc_driver_stop_motors(void)
{
   pthread_mutex_lock(&mutex);
   if (started)
   {
      memset(i2c_setp, HOLGER_I2C_OFF, n_motors);
      sleep(2);
      started = false;
   }
   pthread_mutex_lock(&mutex);
}


bool holger_blmc_driver_write_forces(float forces[4], float voltage, float *rpm)
{
   ASSERT_NOT_NULL(i2c_setp);
   if (!started)
   {
      return false;   
   }
   return force2twi_calc(forces, voltage, i2c_setp);
}



