
/*
 * file: gpio_mosfet.h
 * purpose: driver for ED-Solutions GPIO MOSFET
 * author: Tobias Simon, Ilmenau University of Technology
 */


#ifndef __GPIO_MOSFET_H__
#define __GPIO_MOSFET_H__


#include <stdint.h>

#include "../../lib/i2c/i2c_generic.h"
#include "../../interfaces/gpio.h"


struct gpio_mosfet;
typedef struct gpio_mosfet gpio_mosfet_t;


/*
 * creates and registers mosfet device at i2c bus
 */
gpio_mosfet_t *gpio_mosfet_create(i2c_bus_t *bus, uint8_t addr);


/*
 * destroys mosfet device
 */
void gpio_mosfet_destroy(gpio_mosfet_t *mosfet);


/*
 * sets output of mosfet to value [0, 1]
 */
int gpio_mosfet_set(gpio_mosfet_t *mosfet, gpio_id_t id, uint8_t value);


#endif /* __GPIO_MOSFET_H__ */

