

#include <malloc.h>
#include <string.h>

#include "calibration.h"


void calibration_init(calibration_t *cal, const size_t dim, const size_t max_samples)
{
   cal->sum = malloc(sizeof(float) * dim);
   cal->bias = malloc(sizeof(float) * dim);
   memset(cal->sum, 0, sizeof(float) * dim);
   memset(cal->bias, 0, sizeof(float) * dim);
   cal->dim = dim;
   cal->max_samples = max_samples;
   cal->sample = 0;
}


int calibration_sample_bias(calibration_t *cal, const float *sample)
{
   if (cal->sample != cal->max_samples)
   {
      cal->sample++;
      size_t i;
      for (i = 0; i < cal->dim; i++)
      {
         cal->sum[i] += sample[i];
         cal->bias[i] = cal->sum[i] / (float)cal->sample;
      }
      return 0;
   }
   return 1;
}


void calibration_finalize(calibration_t *cal)
{
   free(cal->sum);
   free(cal->bias);
}

