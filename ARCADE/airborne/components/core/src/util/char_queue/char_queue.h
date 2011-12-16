
/*
 * file: char_queue.h
 *
 * interface to a threadsafe queue data structure for 8-bit unsigned characters
 *
 * author: Tobias Simon, Ilmenau University of Technology
 */


#ifndef __CHAR_QUEUE_H__
#define __CHAR_QUEUE_H__


#include <stdint.h>


/*
 * char queue data structure
 */
typedef struct char_queue char_queue_t;


/*
 * creates a queue of length size
 */
char_queue_t *char_queue_create(uint32_t size);


/*
 * peeks into the queue and returns next entry
 *
 * returns:  0 .. 0xFF if queue is not empty
 *          -1 .. if queue is empty
 */
int32_t char_queue_peek(char_queue_t *queue);


/*
 * removes and returns one char from the queue
 *
 * returns:  0 .. 0xFF if queue is not empty
 *          -1 .. if queue is empty
 */
int32_t char_queue_drain(char_queue_t *queue);


/*
 * appends queue, if enough space is available
 * returns the number of bytes added to the queue
 */
uint32_t char_queue_append(char_queue_t *queue, const uint8_t *data, const uint32_t len);


#endif /* __CHAR_QUEUE_H__ */

