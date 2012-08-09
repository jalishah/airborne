
/*
 * file: holger_blmc.c
 * purpose: mikrokopter brushless motor controller interface
 * author: Tobias Simon, Ilmenau University of Technology
 */


#include <malloc.h>
#include <string.h>
#include <util.h>
#include <periodic_thread.h>

#include "holger_blmc_driver.h"
#include "../../libs/holger_blmc/holger_blmc.h"
#include "../../libs/holger_blmc/force2twi.h"


static bool started = false;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static unsigned int n_motors;
static uint8_t *i2c_setp = NULL;

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



void holger_blmc_driver_init(i2c_bus_t *bus, const uint8_t *addrs, const coupling_t *coupling, const unsigned int _n_motors)
{
   ASSERT_ONCE();
   n_motors = _n_motors;
   force2twi_init(coupling);
   holger_blmc_init(bus, addrs, n_motors);
   i2c_setp = malloc(n_motors);
   memset(i2c_setp, HOLGER_I2C_OFF, n_motors);
   const struct timespec motors_period = {0, RT_MOTORS_PERIOD * NSEC_PER_MSEC};
   periodic_thread_start(&motors_thread, motors_thread_func, "motors_thread",
                         99, motors_period, NULL);
}



void holger_blmc_driver_start_motors(void)
{
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


bool holger_blmc_driver_write_forces(const float forces[4], const float voltage)
{
   ASSERT_NOT_NULL(i2c_setp);
   if (!started)
   {
      return false;   
   }
   return force2twi_calc(forces, voltage, i2c_setp);
}



