
#include <string.h>

#include "util.h"
#include "i2c_generic.h"


void i2c_bus_interface_setup(i2c_bus_interface_t *interface, void (*setup_func)(i2c_bus_interface_t *interface))
{
   ASSERT_NOT_NULL(interface);
   ASSERT_NOT_NULL(setup_func);

   setup_func(interface);
}


void i2c_bus_context_setup(i2c_bus_context_t *context, char *name, char *path)
{
   ASSERT_NOT_NULL(context);
   ASSERT_NOT_NULL(name);
   ASSERT_NOT_NULL(path);

   context->name = name;
   context->path = path;
   pthread_mutex_init(&context->mutex, NULL);
}


void i2c_bus_setup(i2c_bus_t *bus, i2c_bus_interface_t *interface, i2c_bus_context_t *context)
{
   ASSERT_NOT_NULL(bus);
   ASSERT_NOT_NULL(interface);
   ASSERT_NOT_NULL(context);

   bus->interface = interface;
   bus->context = context;
}


int i2c_bus_open(i2c_bus_t *bus)
{
   ASSERT_NOT_NULL(bus);
   ASSERT_NOT_NULL(bus->interface);
   ASSERT_NOT_NULL(bus->interface->open);
   ASSERT_NOT_NULL(bus->context);

   return bus->interface->open(bus->context);
}


int i2c_bus_close(i2c_bus_t *bus)
{
   ASSERT_NOT_NULL(bus);
   ASSERT_NOT_NULL(bus->interface);
   ASSERT_NOT_NULL(bus->interface->close);
   ASSERT_NOT_NULL(bus->context);

   return bus->interface->close(bus->context);
}


void i2c_dev_init(i2c_dev_t *dev, i2c_bus_t *bus, char *name, unsigned char addr)
{
   ASSERT_NOT_NULL(dev);
   ASSERT_NOT_NULL(bus);
   ASSERT_NOT_NULL(name);
   ASSERT_TRUE((addr & 0x80) == 0); /* only allow 7-bit addresses */

   dev->bus = bus;
   dev->name = name;
   dev->addr = addr;
}


void i2c_dev_set_list_terminator(i2c_dev_t *dev)
{
   ASSERT_NOT_NULL(dev);

   dev->bus = NULL;
   dev->name = NULL;
   dev->addr = 0;
}


int i2c_dev_list_end(i2c_dev_t *dev)
{
   ASSERT_NOT_NULL(dev);

   return dev->bus == NULL;
}


int i2c_dev_write(i2c_dev_t *dev, unsigned char cmd, unsigned char val)
{
   ASSERT_NOT_NULL(dev);
   ASSERT_NOT_NULL(dev->bus);
   ASSERT_NOT_NULL(dev->bus->interface);
   ASSERT_NOT_NULL(dev->bus->interface->write_byte);

   return dev->bus->interface->write_byte(dev, cmd, val);
}


int i2c_dev_read(i2c_dev_t *dev, unsigned char cmd)
{
   ASSERT_NOT_NULL(dev);
   ASSERT_NOT_NULL(dev->bus);
   ASSERT_NOT_NULL(dev->bus->interface);
   ASSERT_NOT_NULL(dev->bus->interface->read_byte);

   return dev->bus->interface->read_byte(dev, cmd);
}


int i2c_dev_read_block(i2c_dev_t *dev, unsigned char cmd, unsigned char *buffer, unsigned int len)
{
   ASSERT_NOT_NULL(dev);
   ASSERT_NOT_NULL(dev->bus);
   ASSERT_NOT_NULL(dev->bus->interface);
   ASSERT_NOT_NULL(dev->bus->interface->read_block);

   return dev->bus->interface->read_block(dev, cmd, buffer, len);
}


void i2c_dev_lock_bus(i2c_dev_t *dev)
{
   pthread_mutex_lock(&dev->bus->context->mutex);
}


void i2c_dev_unlock_bus(i2c_dev_t *dev)
{
   pthread_mutex_unlock(&dev->bus->context->mutex);
}

