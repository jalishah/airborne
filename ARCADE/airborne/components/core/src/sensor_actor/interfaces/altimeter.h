/*
 * altimeter.h
 *
 *  Created on: 11.06.2010
 *      Author: tobi
 */


#ifndef ALTIMETER_H
#define ALTIMETER_H


int baro_altimeter_init(void);

float baro_altimeter_read(void);

int ultra_altimeter_init(void);

float ultra_altimeter_read(void);


#endif /* ALTIMETER_H */

