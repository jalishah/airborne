/*
 * interface.h
 *
 *  Created on: 15.06.2010
 *      Author: tobi
 */


#ifndef LEDS_OVERO_H
#define LEDS_OVERO_H


#include "../interfaces/leds.h"


int leds_overo_write(const leds_config_t *config);

int leds_overo_initialize(void);

int leds_overo_finalize(void);


#endif /* LEDS_OVERO_H */

