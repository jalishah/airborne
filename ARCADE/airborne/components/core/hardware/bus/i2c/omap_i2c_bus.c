
/*
 * omap_i2c_bus.c
 *
 * created on: 11.06.2010
 * author: tobi
 */


#include "i2c_generic.h"
#include "omap_i2c.h"


#include "omap_i2c_bus.h"


static i2c_bus_interface_t i2c_bus_interface;
static i2c_bus_context_t i2c_bus_context;
static i2c_bus_t i2c_bus;


int omap_i2c_bus_init(void)
{
   i2c_bus_interface_setup(&i2c_bus_interface, omap_i2c_bus_interface_setup);
   i2c_bus_context_setup(&i2c_bus_context, "omap_i2c", "/dev/i2c-3");
   i2c_bus_setup(&i2c_bus, &i2c_bus_interface, &i2c_bus_context);
   return i2c_bus_open(&i2c_bus);
}


i2c_bus_t *omap_i2c_bus_get(void)
{
   return &i2c_bus;
}

