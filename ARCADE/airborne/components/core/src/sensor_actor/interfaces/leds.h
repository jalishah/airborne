/*
 * leds.h
 *
 * Created on: 14.02.2011
 * Author: tobi
 */


#ifndef LEDS_H
#define LEDS_H


typedef struct
{
   int time_on; /* in ms */
   int time_off; /* in ms */
}
led_config_t;


enum
{
   LEDS_BOTTOM,

   LEDS_COUNT /* pseudo-value, must always be last in enum! */
};


typedef struct
{
   led_config_t data[LEDS_COUNT];
}
leds_config_t;


typedef struct
{
   int (*write)(const leds_config_t *config);
   int (*initialize)(void);
   int (*finalize)(void);
   void (*wait_for_event)(void);
}
leds_device_t;


#endif /* LEDS_H */

