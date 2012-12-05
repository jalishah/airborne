
#ifndef __FLIGHT_DETECT_H__
#define __FLIGHT_DETECT_H__


int flight_detect_init(size_t dimension, size_t window, float treshold, float *weights);


int flight_detect(float *in);


#endif /* __FLIGHT_DETECT_H__ */

