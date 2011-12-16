

#include "gpio.h"
#include "../lib/gpio/gpio_mosfet.h"
#include "../lib/gpio/gpio_mosfet_singleton.h"


int gpio_init(void)
{
   return 0;
}


int gpio_set(gpio_id_t id, uint8_t value)
{
   return gpio_mosfet_set(gpio_mosfet_singleton(), id, value);
}

