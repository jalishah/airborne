
/*
 * powerctrl.h
 *
 * Created on: 14.02.2011
 * Author: tobi
 */


#ifndef POWERCTRL_H
#define POWERCTRL_H


#include "../../util/types.h"


typedef struct
{
   bool_t main_power_on;
}
powerctrl_config_t;


typedef struct
{
   int (*write)(const powerctrl_config_t *config);
   int (*initialize)(void);
   int (*finalize)(void);
   void (*wait_for_event)(void);
}
powerctrl_device_t;


#endif /* POWERCTRL_H */

