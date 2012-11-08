

#include <malloc.h>

#include "motors.h"


motors_interface_t *motors_interface_create(int (*write)(float forces[4], float voltage))
{
   motors_interface_t *i = malloc(sizeof(motors_interface_t));
   i->write = write;
   return i;
}


