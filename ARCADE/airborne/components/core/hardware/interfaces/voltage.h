
#ifndef __VOLTAGE_INTERFACE_H__
#define __VOLTAGE_INTERFACE_H__


typedef struct
{
   float (*read)(void);
}
voltage_interface_t;


voltage_interface_t *voltage_interface_create(float (*read)(void));


#endif /* __VOLTAGE_INTERFACE_H__ */

