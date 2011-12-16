
#ifndef __PARAMS_H__
#define __PARAMS_H__


#include "../util/threads/threadsafe_types.h"


/*
 * initializes parameters access module
 */
void params_init(void);


/*
 * register parameter
 * ! must be called after params_init and before params_thread_start
 *
 * returns:
 *   0, if no error occured
 *  -1, if param was registered previously
 */
int param_add(char *key, threadsafe_float_t *data);


/*
 * starts params request handler thread
 */
void params_thread_start(void);


#endif /* __PARAMS_H__ */

