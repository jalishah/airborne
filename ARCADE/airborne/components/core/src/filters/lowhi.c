#include "filter.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#ifndef M_PI
#define M_PI    3.14159265358979323846f
#endif

void filter_lp_init(Filter2nd *filter, const float fg, const float d, const float Ts, const int signal_dim)
{
    const float T = 1/(2*M_PI*fg);
    
    filter->Ts = Ts;
    filter->signal_dim = signal_dim;
    
    filter->z1 = (float *)calloc(signal_dim,sizeof(float)); 
    filter->z2 = (float *)calloc(signal_dim,sizeof(float));
    
    int i;
    for (i = 0; i<signal_dim; i++)
    {
        filter->z1[i] = 0;
        filter->z2[i] = 0;
    }
    
    filter->a1 = (2*Ts*Ts -8*T*T)/(4*T*T + 4*d*T*Ts + Ts*Ts);
    filter->a2 = (4*T*T - 4*d*T*Ts + Ts*Ts)/(4*T*T + 4*d*T*Ts + Ts*Ts);
    
    filter->b  = Ts*Ts/(4*T*T + 4*d*T*Ts + Ts*Ts);
}

void filter_hp_init(Filter2nd *filter, const float fg, const float d, const float Ts, const int signal_dim)
{
    const float T = 1/(2*M_PI*fg);
    
    filter->Ts = Ts;
    filter->signal_dim = signal_dim;
    
    filter->z1 = (float *)calloc(signal_dim,sizeof(float)); 
    filter->z2 = (float *)calloc(signal_dim,sizeof(float));
    
    int i;
    for (i = 0; i<signal_dim; i++)
    {
        filter->z1[i] = 0;
        filter->z2[i] = 0;
    }
    
    filter->a1 = (2*Ts*Ts -8*T*T)/(4*T*T + 4*d*T*Ts + Ts*Ts);
    filter->a2 = (4*T*T - 4*d*T*Ts + Ts*Ts)/(4*T*T + 4*d*T*Ts + Ts*Ts);
    
    filter->b  = (2*Ts)/(4*T*T + 4*d*T*Ts + Ts*Ts);
}

void filter_hpd_init(Filter2nd *filter, const float fg, const float d, const float Ts, const int signal_dim)
{
    const float T = 1/(2*M_PI*fg);
    
    filter->Ts = Ts;
    filter->signal_dim = signal_dim;
    
    filter->z1 = (float *)calloc(signal_dim,sizeof(float)); 
    filter->z2 = (float *)calloc(signal_dim,sizeof(float));
    
    int i;
    for (i = 0; i<signal_dim; i++)
    {
        filter->z1[i] = 0;
        filter->z2[i] = 0;
    }
    
    filter->a1 = (2*Ts*Ts -8*T*T)/(4*T*T + 4*d*T*Ts + Ts*Ts);
    filter->a2 = (4*T*T - 4*d*T*Ts + Ts*Ts)/(4*T*T + 4*d*T*Ts + Ts*Ts);
    
    filter->b  = 4/(4*T*T + 4*d*T*Ts + Ts*Ts);
}

void filter_init(Filter2ndFull *filter, const float *a, const float *b, const float Ts)
{
    filter->Ts = Ts;

    filter->z1 = 0;
    filter->z2 = 0;
    
    filter->a1 = a[0];
    filter->a2 = a[1];
    
    filter->b0 = b[0];
    filter->b1 = b[1];
    filter->b2 = b[2];
}

void filter_lp_run(Filter2nd *filter, const float *u_in, float *y)
{
    int i;
    float u;
    for (i=0; i<filter->signal_dim; i++)
    {
        u = u_in[i];
        y[i]          =   filter->b*u                   + filter->z1[i];
        filter->z1[i] = 2*filter->b*u - filter->a1*y[i] + filter->z2[i];
        filter->z2[i] =   filter->b*u - filter->a2*y[i];
    }
}

void filter_hp_run(Filter2nd *filter, const float *u_in, float *y)
{
    int i;
    float u;
    for (i=0; i<filter->signal_dim; i++)
    {
        u = u_in[i];
        y[i]          =   filter->b*u                   + filter->z1[i];
        filter->z1[i] =               - filter->a1*y[i] + filter->z2[i];
        filter->z2[i] = - filter->b*u - filter->a2*y[i];
    }
}

void filter_hpd_run(Filter2nd *filter, const float *u_in, float *y)
{
    int i;
    float u;
    for (i=0; i<filter->signal_dim; i++)
    {
        u = u_in[i];
        y[i]          =     filter->b*u                   + filter->z1[i];
        filter->z1[i] = - 2*filter->b*u - filter->a1*y[i] + filter->z2[i];
        filter->z2[i] =     filter->b*u - filter->a2*y[i];
    }
}

const float filter_run(Filter2ndFull *filter, float u)
{
    const float y = filter->b0*u                + filter->z1;
    filter->z1    = filter->b1*u - filter->a1*y + filter->z2;
    filter->z2    = filter->b2*u - filter->a2*y;
    return y;
}
