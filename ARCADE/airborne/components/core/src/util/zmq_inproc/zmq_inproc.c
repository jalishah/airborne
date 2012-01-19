
/*
 * message-queue-based thread communication
 * implementation using zeromq
 */


#include <zmq.h>
#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#include "zmq_inproc.h"


static void *context = NULL;
static GHashTable *sockets_ht;


typedef struct
{
   void *sockets[2];
   char *path;
} 
socket_pair_t;


int inproc_init(void *_context)
{
   if (context == NULL)
   {
      context = _context;
      sockets_ht = g_hash_table_new(g_str_hash, g_str_equal);
      return 0;
   }
   return -1;
}


void inproc_fini(void)
{
   GHashTableIter iter;
   g_hash_table_iter_init(&iter, sockets_ht);
   socket_pair_t *pair;
   while (g_hash_table_iter_next(&iter, NULL, (gpointer)&pair))
   {
      zmq_close(pair->sockets[0]);
      zmq_close(pair->sockets[1]);
      free(pair->path);
      free(pair);
   }
}


void *inproc_create_socket(char *queue)
{
   void *socket = NULL;
   
   socket_pair_t *pair = (socket_pair_t *)g_hash_table_lookup(sockets_ht, queue);
   if (pair == NULL)
   {
      /* create first socket and other structures: */
      pair = malloc(sizeof(socket_pair_t));
      socket = zmq_socket(context, ZMQ_PAIR);
      pair->sockets[0] = socket;
      pair->sockets[1] = NULL;
      pair->path = malloc(strlen(queue) + 10);
      sprintf(pair->path, "inproc://%s", queue);
      zmq_bind(socket, pair->path);
      g_hash_table_insert(sockets_ht, queue, pair);
   }
   else if (pair->sockets[1] == NULL)
   {
      /* create second socket: */
      socket = zmq_socket(context, ZMQ_PAIR);
      pair->sockets[1] = socket;
      zmq_connect(socket, pair->path);
   }

   return socket;
}


char *inproc_read(void *socket, size_t *len, int poll)
{
   char *buffer = NULL;
   size_t _len = 0;
   
   zmq_msg_t message;
   zmq_msg_init(&message);
   int result = zmq_recv(socket, &message, poll ? ZMQ_NOBLOCK : 0);
   if (result == 0)
   {
      _len = zmq_msg_size(&message);
      buffer = malloc(_len);
      memcpy(buffer, zmq_msg_data(&message), _len);
   }
   
   if (len != NULL)
   {
      *len = _len;
   }
   zmq_msg_close(&message);
   
   return buffer;
}


static void simple_free(void *data, void *hint)
{
   (void)hint;
   free(data);
}


int inproc_write(void *socket, char *data, size_t len, int free_str)
{
   zmq_msg_t message;
   zmq_msg_init_data(&message, data, len, free_str ? simple_free : NULL, NULL);
   int ret = zmq_send(socket, &message, 0);
   zmq_msg_close(&message);
   return ret;
}

