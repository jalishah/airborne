
#include "average.h"


void avg_init(avg_data_t *avg_data, int max)
{
   avg_data->avg = 0.0;
   avg_data->count = 0.0;
   avg_data->max_count = max;
   avg_data->sum = 0.0;
}


void avg_add(avg_data_t *avg_data, float value)
{
   /*
    * compute average only if
    * maximum sample count not reached:
    */
   if (avg_data->count < avg_data->max_count)
   {
      avg_data->sum += value;
      avg_data->count++;
      avg_data->avg = avg_data->sum / (float)avg_data->count;
   }
}

