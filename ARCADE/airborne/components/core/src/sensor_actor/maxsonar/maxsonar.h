
#ifndef __MAXSONAR_H__
#define __MAXSONAR_H__


#include <stdint.h>


struct maxsonar;
typedef struct maxsonar maxsonar_t;


maxsonar_t *maxsonar_create(void);

int maxsonar_parse(maxsonar_t *sonar, uint8_t b);

float maxsonar_get_dist(maxsonar_t *sonar);


#endif /* __MAXSONAR_H__ */

