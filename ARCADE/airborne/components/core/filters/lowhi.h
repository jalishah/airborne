
/*
 * purpose: various filters
 * authors: Alexander Barth; Benjamin Jahn; Tobias Simon, Ilmenau University of Technology
 */


#ifndef __FILTER_LOWHI_H__
#define __FILTER_LOWHI_H__



/* type definitions */
typedef struct 
{
    float ts;
    int signal_dim;
    float *z1;
    float *z2;
    float a1;
    float a2;
    float b;
}
filt2nd_t;

typedef struct 
{
    float ts;
    
    /* filter memories: */
    float z1;
    float z2;
    
    /* filter coefficients: */
    float a1;
    float a2;
    float b0;
    float b1;
    float b2;
}
filt2nd_full_t;


void filter_lp_init(filt2nd_t *filt, const float fg, const float damping, const float ts, const int dim);
void filter_lp_run(filt2nd_t *filt, const float *u, float *y);

void filter_hp_init(filt2nd_t *filt, const float fg, const float damping, const float ts, const int dim);
void filter_hp_run(filt2nd_t *filt, const float *u, float *y);

void filter_hpd_init(filt2nd_t *filt, const float fg, const float damping, const float ts, const int dim);
void filter_hpd_run(filt2nd_t *filt, const float *u, float *y);

void filter_full_init(filt2nd_full_t *filt, const float *a, const float *b, const float ts);
float filter_full_run(filt2nd_full_t *filt, const float u);


#endif /* __FILTER_LOWHI_H__ */

