
#ifndef I2C_GENERIC_H
#define I2C_GENERIC_H


#include <pthread.h>


struct i2c_bus;


typedef struct
{
   char *name;
   unsigned char addr;
   struct i2c_bus *bus;
}
i2c_dev_t;


typedef struct
{
   char *name;
   char *path;
   void *priv;
   pthread_mutex_t mutex;
}
i2c_bus_context_t;


typedef struct
{
   int (*open)(i2c_bus_context_t *bus_context);
   int (*close)(i2c_bus_context_t *bus_context);
   int (*write_simple)(i2c_dev_t *dev, unsigned char val);
   int (*write_byte)(i2c_dev_t *dev, unsigned char cmd, unsigned char val);
   int (*read_byte)(i2c_dev_t *dev, unsigned char cmd);
   int (*read_block)(i2c_dev_t *dev, unsigned char cmd, unsigned char *buffer, unsigned int len);
}
i2c_bus_interface_t;


typedef struct i2c_bus
{

   i2c_bus_interface_t *interface;
   i2c_bus_context_t *context;
}
i2c_bus_t;


void i2c_bus_interface_setup(i2c_bus_interface_t *interface, void (*setup_func)(i2c_bus_interface_t *interface));

void i2c_bus_context_setup(i2c_bus_context_t *context, char *name, char *path);

void i2c_bus_setup(i2c_bus_t *bus, i2c_bus_interface_t *interface, i2c_bus_context_t *context);

int i2c_bus_open(i2c_bus_t *bus);

int i2c_bus_close(i2c_bus_t *bus);

void i2c_dev_init(i2c_dev_t *dev, i2c_bus_t *bus, char *name, unsigned char addr);

void i2c_dev_set_list_terminator(i2c_dev_t *dev);

int i2c_dev_list_end(i2c_dev_t *dev);

int i2c_dev_write_simple(i2c_dev_t *dev, unsigned char val);

int i2c_dev_write(i2c_dev_t *dev, unsigned char cmd, unsigned char val);

int i2c_dev_read(i2c_dev_t *dev, unsigned char cmd);

int i2c_dev_read_block(i2c_dev_t *dev, unsigned char cmd, unsigned char *buffer, unsigned int len);

void i2c_dev_lock_bus(i2c_dev_t *dev);

void i2c_dev_unlock_bus(i2c_dev_t *dev);


#endif /* I2C_GENERIC_H */

