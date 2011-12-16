
/*
 * file: gpio_mosfet.c
 * purpose: driver for ED-Solutions GPIO MOSFET
 * author: Tobias Simon, Ilmenau University of Technology
 */


#include <malloc.h>
#include <pthread.h>

#include "util.h"
#include "gpio_mosfet.h"


/*
 * mosfet gpio structure, holds i2c device
 * and current mosfet output status
 */
struct gpio_mosfet
{
   i2c_dev_t device;
   uint8_t status;
   pthread_mutex_t mutex;
};


/*
 * creates and registers mosfet device at i2c bus
 */
gpio_mosfet_t *gpio_mosfet_create(i2c_bus_t *bus, uint8_t addr)
{
   ASSERT_NOT_NULL(bus);
   ASSERT_TRUE((addr & 0x80) == 0);

   gpio_mosfet_t *mosfet = malloc(sizeof(gpio_mosfet_t));
   i2c_dev_init(&mosfet->device, bus, "gpio_mosfet", addr);
   mosfet->status = 0; /* all lines are off by default */
   pthread_mutex_init(&mosfet->mutex, NULL);
   return mosfet;
}


/*
 * destroys mosfet device
 */
void gpio_mosfet_destroy(gpio_mosfet_t *mosfet)
{
   free(mosfet);
}


/*
 * sets output of mosfet to value [0, 1]
 */
int gpio_mosfet_set(gpio_mosfet_t *mosfet, gpio_id_t id, uint8_t value)
{
   ASSERT_NOT_NULL(mosfet);
   ASSERT_TRUE(value < 2);

   /* calculate bit, based on mosfet output id: */
   uint8_t bit = 0;
   switch (id)
   {
      case GPIO_POWER:
         bit = 0x01; /* first line */
         break;

      case GPIO_LEDS:
         bit = 0x04; /* third line */
   }

   /* update status and write to mosfet device: */
   pthread_mutex_lock(&mosfet->mutex);
   if (value)
   {
      mosfet->status |= bit;
   }
   else
   {
      mosfet->status &= ~bit;
   }
   int status = i2c_dev_write(&mosfet->device, 0x01 /* set outputs non-persistent */, mosfet->status);
   pthread_mutex_unlock(&mosfet->mutex);

   return status;
}



