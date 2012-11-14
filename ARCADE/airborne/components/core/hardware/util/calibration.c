

#include <malloc.h>
#include <string.h>

#include <util.h>

#include "calibration.h"


void cal_init(calibration_t *cal, const size_t dim, const size_t max_samples)
{
   ASSERT_NOT_NULL(cal);
   ASSERT_TRUE(dim > 0);
   ASSERT_TRUE(max_samples > 0);

   cal->sum = malloc(sizeof(float) * dim);
   cal->bias = malloc(sizeof(float) * dim);
   cal->dim = dim;
   cal->max_samples = max_samples;
   cal_reset(cal);
}


void cal_reset(calibration_t *cal)
{
   memset(cal->sum, 0, sizeof(float) * cal->dim);
   cal->sample = 0;
}


static int calibration_sample_bias(calibration_t *cal, const float *sample)
{
   ASSERT_NOT_NULL(cal);
   ASSERT_NOT_NULL(sample);

   if (cal->sample != cal->max_samples)
   {
      cal->sample++;
      size_t i;
      FOR_N(i, cal->dim)
      {
         cal->sum[i] += sample[i];
         cal->bias[i] = cal->sum[i] / (float)cal->sample;
      }
      return 0;
   }
   return 1;
}


int cal_sample_apply(calibration_t *cal, float *vec)
{
   if (calibration_sample_bias(cal, vec) == 1)
   {
      FOR_N(i, cal->dim)
      {
         vec[i] -= cal->bias[i];
      }
      return 1;
   }
   return 0;
}

