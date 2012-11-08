
#ifndef __CALIBRATION_H__
#define __CALIBRATION_H__


typedef struct
{
   size_t dim; /* dimension of the calibration data */
   float *sum;
   float *bias;
   size_t max_samples;
   size_t sample;
}
calibration_t;


void calibration_init(calibration_t *cal, const size_t dim, const size_t max_samples);


int calibration_sample_bias(calibration_t *cal, const float *sample);


void calibration_finalize(calibration_t *cal);


#endif /* __CALIBRATION_H__ */

