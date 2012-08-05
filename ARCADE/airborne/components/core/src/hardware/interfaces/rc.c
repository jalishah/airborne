

#include <malloc.h>

#include "rc.h"


rc_interface_t *rc_interface_create(int (*init)(void), void (*read)(rc_data_t *data))
{
   rc_interface_t *interface = malloc(sizeof(rc_interface_t));
   interface->init = init;
   interface->read = read;
   return interface;
}


int rc_init(rc_interface_t *interface)
{
   return interface->init();
}


void rc_read(rc_interface_t *interface, rc_data_t *data)
{
   return interface->read(data);   
}

