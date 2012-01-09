
/*
 * opcd params implementation
 * author: Tobias Simon, Ilmenau University of Technology
 */


#ifndef __OPCD_PARAMS_H__
#define __OPCD_PARAMS_H__


typedef struct
{
   char *id;
   void *data;
}
opcd_param_t;


#define OPCD_PARAMS_END {NULL, NULL}

/*
 * initialize OPCD params
 * prefix is added before all keys defined in the opcd_param_t's
 */
void opcd_params_init(char *prefix);


/*
 * sets a float param to identfied by id to val
 */
void opcd_float_param_set(char *id, float val);


/*
 * 1) retrieves the configuration value from OPCD and store this value
 * 2) registers the parameter for online updates
 */
void opcd_params_apply(char *prefix, opcd_param_t *params);


#endif /* __OPCD_PARAMS__ */

