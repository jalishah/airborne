/*
 * median_filter.c
 *
 *  Created on: 18.06.2010
 *      Author: tobi
 */


#include <stdlib.h>
#include <string.h>

#include "median_filter.h"
#include "../util/util.h"



void median_filter_init(median_filter_t *filter, float *history, float *sort, size_t size)
{
   filter->history = history;
   filter->sort = sort;
   filter->size = size;
   filter->count = 0;
}


int median_filter_ready(median_filter_t *filter)
{
   return filter->count == filter->size;
}


float median_filter_run(median_filter_t *filter, float val)
{
   unsigned int i;
   for (i = 0; i < filter->size - 1; i++)
   {
      filter->history[i] = filter->history[i + 1];
   }
   filter->history[i] = val;
   if (filter->count < filter->size)
   {
      filter->count++;
   }
   memcpy(filter->sort, filter->history, sizeof(float) * filter->size);
   qsort(filter->sort, filter->size, sizeof(float), compare_floats);
   return filter->sort[filter->size / 2 + ((filter->size & 1) ? 1 : 0)];
}
