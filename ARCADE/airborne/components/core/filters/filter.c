#include "filter.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#ifndef M_PI
#define M_PI    3.14159265358979323846f
#endif

// FIRST ORDER

void filter1_lp_init(Filter1 *filter, float fg, float Ts, int signal_dim)
{
   /* calculate Filter Time constant */
   float T = 1/(2*M_PI*fg);

   /* Filter Coefficients for 2nd order highpass filter */
   float a[1];
   a[0] = (2*Ts)/(2*T + Ts) - 1;

   float b[2];
   b[0] = Ts/(2*T + Ts);
   b[1] = b[0];
   filter1_init(filter, a, b, Ts, signal_dim);
}

void filter1_hp_init(Filter1 *filter, float fg, float Ts, int signal_dim)
{
   /* calculate Filter Time constant */
   float T = 1/(2*M_PI*fg);

   /* Filter Coefficients for 2nd order highpass filter */
   float a[1];
   a[0] = (2*Ts)/(2*T + Ts) - 1;

   float b[2];
   b[0] = 2/(2*T + Ts);
   b[1] = -b[0];
   filter1_init(filter, a, b, Ts, signal_dim);
}


void filter1_init(Filter1 *filter, float *a, float *b, float Ts, int signal_dim)
{
   int i=0;
   filter->Ts = Ts;
   filter->signal_dim = signal_dim;
    
   filter->z = (float *)calloc(signal_dim,sizeof(float)); 

   for (i=0; i<signal_dim; i++)
   {
      filter->z[i] = 0;
   }
   filter->Ts = Ts;
   filter->a1 = a[0];
    
   filter->b0 = b[0];
   filter->b1 = b[1];
}

void filter1_run(Filter1 *filter, float *u_in, float *y)
{
   int i;
   float u;
   for (i=0; i<filter->signal_dim; i++)
   {
      u = u_in[i];
      y[i]         = filter->b0*u + filter->z[i];
      filter->z[i] = filter->b1*u - filter->a1*y[i];
   }
}

// SECOND ORDER

void filter2_lp_init(Filter2 *filter, float fg, float d, float Ts, int signal_dim)
{
   /* calculate Filter Time constant */
   float T = 1/(2*M_PI*fg);

   /* Filter Coefficients for 2nd order highpass filter */
   float a[2];
   a[0] = (2*Ts*Ts -8*T*T)/(4*T*T + 4*d*T*Ts + Ts*Ts);
   a[1] = (4*T*T - 4*d*T*Ts + Ts*Ts)/(4*T*T + 4*d*T*Ts + Ts*Ts);

   float b[3];
   b[0] = Ts*Ts/(4*T*T + 4*d*T*Ts + Ts*Ts);
   b[1] = 2*b[0];
   b[2] = b[0];
   filter2_init(filter, a, b, Ts, signal_dim);
}

void filter2_hp_init(Filter2 *filter, float fg, float d, float Ts, int signal_dim)
{
   /* calculate Filter Time constant */
   float T = 1/(2*M_PI*fg);

   /* Filter Coefficients for 2nd order highpass filter */
   float a[2];
   a[0] = (2*Ts*Ts -8*T*T)/(4*T*T + 4*d*T*Ts + Ts*Ts);
   a[1] = (4*T*T - 4*d*T*Ts + Ts*Ts)/(4*T*T + 4*d*T*Ts + Ts*Ts);

   float b[3];
   b[0] = (2*Ts)/(4*T*T + 4*d*T*Ts + Ts*Ts);
   b[1] = 0.0f;
   b[2] = -b[0];
   filter2_init(filter, a, b, Ts, signal_dim);
}


void filter2_init(Filter2 *filter, float *a, float *b, float Ts, int signal_dim)
{
   int i=0;
   filter->Ts = Ts;
   filter->signal_dim = signal_dim;
    
   filter->z1 = (float *)calloc(signal_dim,sizeof(float)); 
   filter->z2 = (float *)calloc(signal_dim,sizeof(float));

   for (i=0; i<signal_dim; i++)
   {
      filter->z1[i] = 0;
      filter->z2[i] = 0;
   }
   filter->Ts = Ts;
   filter->a1 = a[0];
   filter->a2 = a[1];
    
   filter->b0 = b[0];
   filter->b1 = b[1];
   filter->b2 = b[2];
}

void filter2_run(Filter2 *filter, float *u_in, float *y)
{
   int i;
   float u;
   for (i=0; i<filter->signal_dim; i++)
   {
      u = u_in[i];
      y[i]          = filter->b0*u                   + filter->z1[i];
      filter->z1[i] = filter->b1*u - filter->a1*y[i] + filter->z2[i];
      filter->z2[i] = filter->b2*u - filter->a2*y[i];
   }
}

