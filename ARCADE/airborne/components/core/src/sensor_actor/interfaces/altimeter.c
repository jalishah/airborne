/*
 * altimeter.c
 *
 *  Created on: 11.06.2010
 *      Author: tobi
 */


#include "altimeter.h"
#include "../holger_fc/holger_fc.h"
#include "../maxsonar/maxsonar_reader.h"


int baro_altimeter_init(void)
{
   return fc_init();   
}


float baro_altimeter_read(void)
{
   return fc_read_alt();   
}


int ultra_altimeter_init(void)
{
   return maxsonar_reader_init();
}


float ultra_altimeter_read(void)
{
   return maxsonar_reader_get_alt();
}

