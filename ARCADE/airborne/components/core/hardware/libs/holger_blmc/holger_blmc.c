
/*
 * file: holger_blmc.c
 * purpose: mikrokopter brushless motor controller interface
 * author: Tobias Simon, Ilmenau University of Technology
 */


#include <fcntl.h>
#include <stdio.h>
#include <malloc.h>


#include "holger_blmc.h"


static i2c_dev_t *devices;
static unsigned int n_motors;


void holger_blmc_init(i2c_bus_t *bus, const uint8_t *addrs, const unsigned int _n_motors)
{
   /* allocate memory for device array: */
   devices = malloc(n_motors * sizeof(i2c_dev_t));

   /* initialize devices: */
   for (int i = 0; i < n_motors; i++)
   {
      i2c_dev_init(&devices[i], bus, "blmc", addrs[i]);
   }
   n_motors = _n_motors;
}


void holger_blmc_write(uint8_t *setpoints)
{
   /* write setpoints: */
   for (int i = 0; i < n_motors; i++)
   {
      i2c_dev_write(&devices[i], 0, setpoints[i]);
   }
}

