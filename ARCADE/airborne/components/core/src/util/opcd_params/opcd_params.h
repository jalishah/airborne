
/*
 * online parameter configuration client interface
 */


#ifndef __OPC_INTERFACE_H__
#define __OPC_INTERFACE_H__


typedef struct
{
   char *id;
   void *data;
}
opcd_param_t;


#define OPCD_PARAMS_END {NULL, NULL}


void opcd_params_init(void);

/*
 * 1) retrieves the configuration value from OPCD and store this value
 * 2) registers the parameter for online updates
 */
void opcd_params_apply(opcd_param_t *params);


#endif /* __OPC_INTERFACE_H__ */

