
/*
 * message-queue-based thread communication
 * interface
 */


#ifndef __INPROC_H__
#define __INPROC_H__


#include <stddef.h>


int inproc_init(void *context);

void inproc_fini(void);

void *inproc_create_socket(char *queue);

char *inproc_read(void *socket, size_t *len, int poll);

int inproc_write(void *socket, char *data, size_t len, int free_str);


#endif /* __INPROC_H__ */

