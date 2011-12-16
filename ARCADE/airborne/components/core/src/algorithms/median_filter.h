/*
 * median_filter.h
 *
 *  Created on: 18.06.2010
 *      Author: tobi
 */

#ifndef MEDIAN_FILTER_H
#define MEDIAN_FILTER_H


#include <stddef.h>


typedef struct
{
   float *history;
   float *sort;
   size_t size;
   size_t count;
}
median_filter_t;


void median_filter_init(median_filter_t *filter, float *history, float *sort, size_t size);

int median_filter_ready(median_filter_t *filter);

float median_filter_run(median_filter_t *filter, float val);


#endif /* MEDIAN_FILTER_H */
