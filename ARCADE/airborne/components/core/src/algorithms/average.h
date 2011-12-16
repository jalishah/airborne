
#ifndef AVERAGE_H
#define AVERAGE_H


typedef struct
{
   float sum;
   float avg;
   int count;
   int max_count;
}
avg_data_t;


#define AVG_DATA_INITIALIZER(max) {0, 0, 0, max}

void avg_init(avg_data_t *avg_data, int max);

void avg_add(avg_data_t *avg_data, float value);


#endif /* AVERAGE_H */

