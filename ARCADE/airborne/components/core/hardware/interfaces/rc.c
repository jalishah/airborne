

#include <malloc.h>

#include "rc.h"


rc_interface_t *rc_interface_create(int (*read)(float channels[MAX_CHANNELS]))
{
   rc_interface_t *interface = malloc(sizeof(rc_interface_t));
   interface->read = read;
   return interface;
}

