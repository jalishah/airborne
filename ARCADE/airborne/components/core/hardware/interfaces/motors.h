

#ifndef __MOTORS_INTERFACE_H__


#include <stdbool.h>


typedef struct
{
   int (*start)(void);
   void (*stop)(void);
   bool (*write)(float forces[4], float voltage, float *rpm);
   unsigned int count;
}
motors_interface_t;


motors_interface_t *motors_interface_create(unsigned int count, int (*start)(void), void (*stop)(void), bool (*write)(float forces[4], float voltage, float *rpm));


#endif /* __MOTORS_INTERFACE_H__ */

