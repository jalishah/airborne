

#ifndef __GPIO_H__
#define __GPIO_H__


#include <stdint.h>


typedef enum
{
   GPIO_POWER,
   GPIO_LEDS
}
gpio_id_t;


int gpio_init(void);

int gpio_set(gpio_id_t id, uint8_t value);


#endif /* __GPIO_H__ */

