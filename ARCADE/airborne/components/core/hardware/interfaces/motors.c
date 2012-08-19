

#include <malloc.h>

#include "motors.h"


motors_interface_t *motors_interface_create(unsigned int count, int (*start)(void), void (*stop)(void), bool (*write)(float forces[4], float voltage, float *rpm))
{
   motors_interface_t *i = malloc(sizeof(motors_interface_t));
   i->count = count;
   i->start = start;
   i->stop = stop;
   i->write = write;
   return i;
}


