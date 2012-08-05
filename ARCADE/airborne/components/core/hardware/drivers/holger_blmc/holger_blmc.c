
/*
 * file: mk_blmc.c
 * purpose: mikrokopter brushless motor controller interface
 * author: Tobias Simon
 */


#include "mk_blmc.h"

#include "../../../util/i2c-dev.h"
#include <fcntl.h>
#include <stdio.h>
#include <malloc.h>


struct mk_blmc
{
   int file;
   uint8_t *addr_table;
   size_t n_motors;
};


mk_blmc_t *mk_blmc_create(uint8_t adapter, uint8_t *addr_table, size_t n_motors)
{
   mk_blmc_t *motors = malloc(sizeof(mk_blmc_t));
   char buffer[32];
   sprintf(buffer, "/dev/i2c-%d", adapter);
   motors->file = open(buffer, O_RDWR);
   if (motors->file < 0)
   {
      return NULL;
   }
   motors->addr_table = addr_table;
   motors->n_motors = n_motors;
   return motors;
}


void mk_blmc_write(mk_blmc_t *motors, uint8_t *setpoints)
{
   for (int i = 0; i < motors->n_motors; i++)
   {
      ioctl(motors->file, I2C_SLAVE, motors->addr_table[i]);
      i2c_smbus_write_byte(motors->file, setpoints[i]);
   }
}

