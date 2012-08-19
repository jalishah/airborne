
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <malloc.h>


#include "omap_i2c.h"
#include "util.h"
#include "i2c-dev.h"


typedef struct
{
   int handle;
   int current_addr;
}
omap_bus_data_t;


static int omap_i2c_bus_open(i2c_bus_context_t *bus_context)
{
   ASSERT_NOT_NULL(bus_context);
   ASSERT_NOT_NULL(bus_context->path);
   ASSERT_NULL(bus_context->priv);

   int bus_handle = open(bus_context->path, O_RDWR);
   if (bus_handle >= 0)
   {
      omap_bus_data_t *data = (omap_bus_data_t *)malloc(sizeof(omap_bus_data_t));
      ASSERT_NOT_NULL(data);

      data->current_addr = -1; /* invalid address, will never be used */
      data->handle = bus_handle;
      bus_context->priv = data;
      return 0;
   }
   return bus_handle; /* should be -1 */
}


static int omap_i2c_bus_close(i2c_bus_context_t *bus_context)
{
   ASSERT_NOT_NULL(bus_context);
   omap_bus_data_t *data = (omap_bus_data_t *)bus_context->priv;
   ASSERT_NOT_NULL(data);

   int status = close(data->handle);
   if (status == 0)
   {
      free(data);
      bus_context->priv = NULL;
   }
   return status;
}


static int omap_i2c_bus_set_slave_address_if_needed(omap_bus_data_t *data, unsigned char addr)
{
   ASSERT_NOT_NULL(data);

   int status = 0;
   if (data->current_addr != addr)
   {
      status = ioctl(data->handle, I2C_SLAVE, addr);
      data->current_addr = addr;
   }
   return status;
}


static int omap_i2c_bus_write_simple(i2c_dev_t *dev, unsigned char val)
{
   ASSERT_NOT_NULL(dev);
   omap_bus_data_t *data = (omap_bus_data_t *)dev->bus->context->priv;
   ASSERT_NOT_NULL(data);
   int status = omap_i2c_bus_set_slave_address_if_needed(data, dev->addr);
   if (status < 0)
   {
      return status;
   }
   return i2c_smbus_write_quick(data->handle, val);
}

static int omap_i2c_bus_write_byte(i2c_dev_t *dev, unsigned char cmd, unsigned char val)
{
   ASSERT_NOT_NULL(dev);

   omap_bus_data_t *data = (omap_bus_data_t *)dev->bus->context->priv;
   ASSERT_NOT_NULL(data);

   int status = omap_i2c_bus_set_slave_address_if_needed(data, dev->addr);
   if (status < 0)
   {
      return status;
   }
   return i2c_smbus_write_byte_data(data->handle, cmd, val);
}


static int omap_i2c_bus_read_byte(i2c_dev_t *dev, unsigned char cmd)
{
   ASSERT_NOT_NULL(dev);

   omap_bus_data_t *data = (omap_bus_data_t *)dev->bus->context->priv;
   ASSERT_NOT_NULL(data);

   int status = omap_i2c_bus_set_slave_address_if_needed(data, dev->addr);
   if (status < 0)
   {
      return status;
   }
   return i2c_smbus_read_byte_data(data->handle, cmd);
}


static int omap_i2c_bus_read_block(i2c_dev_t *dev, unsigned char cmd, unsigned char *buffer, unsigned int len)
{
   (void)len;
   ASSERT_NOT_NULL(dev);

   omap_bus_data_t *data = (omap_bus_data_t *)dev->bus->context->priv;
   ASSERT_NOT_NULL(data);

   int status = omap_i2c_bus_set_slave_address_if_needed(data, dev->addr);
   if (status < 0)
   {
      return status;
   }
   return i2c_smbus_read_block_data(data->handle, cmd, buffer);
}


void omap_i2c_bus_interface_setup(i2c_bus_interface_t *bus_interface)
{
   ASSERT_NOT_NULL(bus_interface);
   ASSERT_NULL(bus_interface->open);
   ASSERT_NULL(bus_interface->close);
   ASSERT_NULL(bus_interface->write_byte);
   ASSERT_NULL(bus_interface->read_byte);
   ASSERT_NULL(bus_interface->read_block);

   bus_interface->open = omap_i2c_bus_open;
   bus_interface->close = omap_i2c_bus_close;
   bus_interface->write_simple = omap_i2c_bus_write_simple;
   bus_interface->write_byte = omap_i2c_bus_write_byte;
   bus_interface->read_byte = omap_i2c_bus_read_byte;
   bus_interface->read_block = omap_i2c_bus_read_block;
}

