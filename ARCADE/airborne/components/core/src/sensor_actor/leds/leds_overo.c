/*
 * interface.h
 *
 *  Created on: 15.06.2010
 *      Author: tobi
 */


#include <string.h>
#include <pthread.h>
#include "leds_overo.h"
#include "../../util/threads/periodic_thread.h"
#include "../lib/gpio/gpio_mosfet_singleton.h"
#include "../lib/i2c/omap_i2c_bus.h"
#include "../../util/time/ltime.h"
#include "util.h"


#define BLINK_THREAD_NAME      "led-blink"
#define BLINK_THREAD_PRIORITY  0
#define BLINK_THREAD_PERIOD    30 /* ms */


static periodic_thread_t blink_thread;


static leds_config_t config = {{{1, 50}}};
static pthread_mutex_t leds_config_mutex = PTHREAD_MUTEX_INITIALIZER;


PERIODIC_THREAD_BEGIN(blink_thread_func)
{
   int state[LEDS_COUNT];
   memset(state, 0, sizeof(state));

   PERIODIC_THREAD_LOOP_BEGIN
   {
      pthread_mutex_lock(&leds_config_mutex);
      for (int i = 0; i < LEDS_COUNT; i++)
      {
         int on_off_time = config.data[i].time_on + config.data[i].time_off;
         state[i] = (state[i] + 1) % on_off_time;
         uint8_t value = (state[i] <= config.data[i].time_on) ? 1 : 0;
         gpio_mosfet_set(gpio_mosfet_singleton(), GPIO_LEDS, value);
      }
      pthread_mutex_unlock(&leds_config_mutex);
   }
   PERIODIC_THREAD_LOOP_END
}
PERIODIC_THREAD_END


int leds_overo_write(const leds_config_t *new_config)
{
   pthread_mutex_lock(&leds_config_mutex);
   config = *new_config;
   pthread_mutex_unlock(&leds_config_mutex);
   return 0;
}


int leds_overo_initialize(void)
{
   const struct timespec blink_period = {0, BLINK_THREAD_PERIOD * NSEC_PER_MSEC};

   periodic_thread_start(&blink_thread, blink_thread_func, BLINK_THREAD_NAME,
                         BLINK_THREAD_PRIORITY, blink_period, NULL);

   return 0;
}


int leds_overo_finalize(void)
{
   periodic_thread_stop(&blink_thread);
   return 0;
}

