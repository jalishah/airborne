

#include <malloc.h>


#include "voltage.h"


voltage_interface_t *voltage_interface_create(float (*read)(void))
{
   voltage_interface_t *i = malloc(sizeof(voltage_interface_t));
   i->read = read;
   return i;
}

