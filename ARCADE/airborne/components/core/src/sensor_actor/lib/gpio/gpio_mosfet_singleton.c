

#include "gpio_mosfet_singleton.h"
#include "../i2c/omap_i2c_bus.h"


static gpio_mosfet_t *mosfet = NULL;


gpio_mosfet_t *gpio_mosfet_singleton(void)
{
   if (mosfet == NULL)
   {
      mosfet = gpio_mosfet_create(omap_i2c_bus_get(), 0x11);
   }
   return mosfet;
}

