
#ifndef __CALIBRATION_H__
#define __CALIBRATION_H__


typedef struct
{
   size_t dim; /* dimension of the calibration data */
   float *sum; /* sum vector */
   float *bias; /* bias vector */
   size_t max_samples; /* maximum number of samples */
   size_t sample; /* current sample counter */
}
calibration_t;


void cal_init(calibration_t *cal, const size_t dim, const size_t max_samples);

void cal_reset(calibration_t *cal);

int cal_sample_apply(calibration_t *cal, float *vec);


#endif /* __CALIBRATION_H__ */

