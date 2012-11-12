
#ifndef __MAXSONAR_H__
#define __MAXSONAR_H__


#include <stdint.h>


struct maxsonar;
typedef struct maxsonar maxsonar_t;


/*
 * creates a maxsonar protocol parser
 */
maxsonar_t *maxsonar_create(void);


/*
 * executes the maxsonar protocol parser
 * returns -1, if parser failed
 *          0, if parser worked
 *          1, if a new packed was parsed
 */
int maxsonar_parse(maxsonar_t *sonar, uint8_t b);


/*
 * returns the current distance parsed
 */
float maxsonar_get_dist(maxsonar_t *sonar);


#endif /* __MAXSONAR_H__ */

