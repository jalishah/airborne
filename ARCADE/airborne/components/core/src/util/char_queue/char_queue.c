
/*
 * file: char_queue.c
 *
 * implementation of a threadsafe queue data structure for 8-bit unsigned characters
 *
 * author: Tobias Simon, Ilmenau University of Technology
 */


#include <malloc.h>
#include <pthread.h>
#include <assert.h>


#include "char_queue.h"


/*
 * private queue data structure:
 */
struct char_queue
{
   uint8_t *data;
   uint32_t size;
   uint32_t read_pos;
   uint32_t write_pos;
   pthread_mutex_t mutex;
};


/*
 * creates a queue of length size
 */
char_queue_t *char_queue_create(uint32_t size)
{
   assert(size != UINT32_MAX);
   /* create queue structure: */
   char_queue_t *queue = malloc(sizeof(char_queue_t));
   /* allocate data: one more to avoid special cases (empty vs. full buffer) */
   queue->data = malloc(size + 1);
   queue->size = size + 1;
   /* initialize read and write positions: */
   queue->read_pos = 0;
   queue->write_pos = 0;
   /* initialize mutex: */
   pthread_mutex_init(&queue->mutex, NULL);
   return queue;
}


/*
 * peeks into the queue and returns next entry
 *
 * returns:  0 .. 0xFF if queue is not empty
 *          -1 .. if queue is empty
 */
int32_t char_queue_peek(char_queue_t *queue)
{
   int32_t ret;
   pthread_mutex_lock(&queue->mutex);
   if (queue->read_pos == queue->write_pos)
   {
      ret = -1;
   }
   else
   {
      ret = queue->data[queue->read_pos];
   }
   pthread_mutex_unlock(&queue->mutex);
   return ret;
}


/*
 * removes and returns one char from the queue
 *
 * returns:  0 .. 0xFF if queue is not empty
 *          -1 .. if queue is empty
 */
int32_t char_queue_drain(char_queue_t *queue)
{
   int32_t ret;
   pthread_mutex_lock(&queue->mutex);
   if (queue->read_pos == queue->write_pos)
   {
      ret = -1;
   }
   else
   {
      ret = queue->data[queue->read_pos];
      queue->read_pos++;
   }
   pthread_mutex_unlock(&queue->mutex);
   return ret;
}


/*
 * appends queue, if enough space is available
 * returns the number of bytes added to the queue
 */
uint32_t char_queue_append(char_queue_t *queue, const uint8_t *data, const uint32_t len)
{
   uint32_t ret = len;
   pthread_mutex_lock(&queue->mutex);
   for (uint32_t i = 0; i < len; i++)
   {
      uint32_t next_write_pos = (queue->write_pos + 1) % queue->size;
      if (next_write_pos == queue->read_pos)
      {
         ret = i;
         break;
      }
      queue->data[queue->write_pos] = data[i];
      queue->write_pos = next_write_pos;
   }
   pthread_mutex_unlock(&queue->mutex);
   return ret;
}


/*
 * free queue
 */
void char_queue_destroy(char_queue_t *queue)
{
   free(queue->data);
   free(queue);
}

