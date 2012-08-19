
/*
 * file: holger_blmc.c
 * purpose: mikrokopter brushless motor controller interface
 * author: Tobias Simon, Ilmenau University of Technology
 */


#include <malloc.h>

#include <util.h>

#include "holger_blmc.h"


static i2c_dev_t *devices = NULL;
static unsigned int n_motors = 0;


void holger_blmc_init(i2c_bus_t *bus, const uint8_t *addrs, const unsigned int _n_motors)
{
   ASSERT_ONCE();
   n_motors = _n_motors;
   /* allocate memory for device array: */
   devices = malloc(n_motors * sizeof(i2c_dev_t));
   /* initialize devices: */
   FOR_N(i, n_motors)
   {
      i2c_dev_init(&devices[i], bus, "blmc", addrs[i]);
   }
}


void holger_blmc_write(uint8_t *setpoints)
{
   ASSERT_NOT_NULL(devices);
   FOR_N(i, n_motors)
   {
      i2c_dev_write(&devices[i], 0, setpoints[i]);
   }
}


void holger_blmc_write_read(uint8_t *setpoints, uint8_t *rpm)
{
   ASSERT_NOT_NULL(devices);
   FOR_N(i, n_motors)
   {
      i2c_dev_write(&devices[i], 0, setpoints[i]);
      rpm[i] = i2c_dev_read(&devices[i], 0);
   }
}

