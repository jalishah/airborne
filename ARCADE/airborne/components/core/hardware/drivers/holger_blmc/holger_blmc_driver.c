
/*
 * file: holger_blmc.c
 * purpose: mikrokopter brushless motor controller interface
 * author: Tobias Simon, Ilmenau University of Technology
 */


#include <fcntl.h>
#include <stdio.h>
#include <malloc.h>


#include "holger_blmc_driver.h"
#include "../../libs/holger_blmc/holger_blmc.h"
#include "../../libs/holger_blmc/force2twi.h"


static i2c_dev_t *devices;
static unsigned int n_motors;
static uint8_t *i2c_setp;



void holger_blmc_driver_init(i2c_bus_t *bus, const uint8_t *addrs, const coupling_t *coupling, const size_t n_motors)
{
   force2twi_init(coupling);
   holger_blmc_init(bus, addrs, n_motors);
   /* allocate memory for device array: */
   devices = malloc(n_motors * sizeof(i2c_dev_t));

   /* initialize devices: */
   for (int i = 0; i < n_motors; i++)
   {
      i2c_dev_init(&devices[i], bus, "blmc", addrs[i]);
   }
}


bool holger_blmc_write_forces(const float forces[4], const float voltage)
{
   bool int_enabled = force2twi_calc(forces, voltage, i2c_setp);
   holger_blmc_write(i2c_setp);
   return int_enabled;
}

