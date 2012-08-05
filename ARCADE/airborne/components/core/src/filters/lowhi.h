
/*
 * purpose: various filters
 * authors: Alexander Barth; Benjamin Jahn; Tobias Simon, Ilmenau University of Technology
 */


#ifndef __FILTER_LOWHI_H__
#define __FILTER_LOWHI_H__



/* type definitions */
typedef struct 
{
    float Ts;
    int signal_dim;
    float *z1;
    float *z2;
    float a1;
    float a2;
    float b;
}
Filter2nd;

typedef struct 
{
    float Ts;
    
    /* Memories */
    float z1;
    float z2;
    
    /* Filter Coefficients */
    float a1;
    float a2;
    float b0;
    float b1;
    float b2;
}
Filter2ndFull;


void filter_lp_init(Filter2nd *filter, const float fg, const float damping, const float Ts, const int dim);
void filter_hp_init(Filter2nd *filter, const float fg, const float damping, const float Ts, const int dim);
void filter_hpd_init(Filter2nd *filter, const float fg, const float damping, const float Ts, const int dim);

void filter_lp_run(Filter2nd *filter, const float *u, float *y);
void filter_hp_run(Filter2nd *filter, const float *u, float *y);
void filter_hpd_run(Filter2nd *filter, const float *u, float *y);

float filter_run(Filter2ndFull *filter, const float u);
void filter_init(Filter2ndFull *filter, const float *a, const float *b, const float Ts);

void filter_term(Filter2nd *filter);


#endif /* __FILTER_LOWHI_H__ */

