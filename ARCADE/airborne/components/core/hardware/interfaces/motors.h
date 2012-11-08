

#ifndef __MOTORS_INTERFACE_H__


#include <stdbool.h>


typedef struct
{
   int (*write)(float forces[4], float voltage);
}
motors_interface_t;


motors_interface_t *motors_interface_create(int (*write)(float forces[4], float voltage));


#endif /* __MOTORS_INTERFACE_H__ */

