
/*
 * purpose: various filters
 * authors: Alexander Barth; Benjamin Jahn; Tobias Simon, Ilmenau University of Technology
 */


#include <math.h>
#include <malloc.h>
#include "lowhi.h"


void filter_lp_init(Filter2nd *filter, const float fg, const float d, const float Ts, const int signal_dim)
{
   const float T = 1.0 / (2.0 * M_PI * fg);
 
   filter->Ts = Ts;
   filter->signal_dim = signal_dim;
 
   filter->z1 = malloc(signal_dim * sizeof(float)); 
   filter->z2 = malloc(signal_dim * sizeof(float));

   for (int i = 0; i < signal_dim; i++)
   {
      filter->z1[i] = 0.0;
      filter->z2[i] = 0.0;
   }

   filter->a1 = (2.0 * Ts * Ts - 8.0 * T * T) / (4.0 * T * T + 4.0 * d * T * Ts + Ts * Ts);
   filter->a2 = (4.0 * T * T - 4.0 * d * T * Ts + Ts * Ts) / (4.0 * T * T + 4.0 * d * T * Ts + Ts * Ts);
   filter->b  = Ts * Ts / (4.0 * T * T + 4.0 * d * T * Ts + Ts * Ts);
}


void filter_hp_init(Filter2nd *filter, const float fg, const float d, const float Ts, const int signal_dim)
{
   const float T = 1.0 / (2.0 * M_PI * fg);
    
   filter->Ts = Ts;
   filter->signal_dim = signal_dim;
    
   filter->z1 = malloc(signal_dim * sizeof(float)); 
   filter->z2 = malloc(signal_dim * sizeof(float));

   for (int i = 0; i < signal_dim; i++)
   {
      filter->z1[i] = 0.0;
      filter->z2[i] = 0.0;
   }

   filter->a1 = (2.0 * Ts * Ts - 8.0 * T * T) / (4.0 * T * T + 4.0 * d * T * Ts + Ts * Ts);
   filter->a2 = (4.0 * T * T - 4.0 * d * T * Ts + Ts * Ts) / (4.0 * T * T + 4.0 * d * T * Ts + Ts * Ts); 
   filter->b  = (2.0 * Ts) / (4.0 * T * T + 4.0 * d * T * Ts + Ts * Ts);
}


void filter_hpd_init(Filter2nd *filter, const float fg, const float d, const float Ts, const int signal_dim)
{
   const float T = 1.0 / (2.0 * M_PI * fg);
    
   filter->Ts = Ts;
   filter->signal_dim = signal_dim;
    
   filter->z1 = malloc(signal_dim * sizeof(float));
   filter->z2 = malloc(signal_dim * sizeof(float));
    
   int i;
   for (i = 0; i < signal_dim; i++)
   {
      filter->z1[i] = 0;
      filter->z2[i] = 0;
   }
    
   filter->a1 = (2.0 * Ts * Ts - 8.0 * T * T) / (4.0 * T * T + 4.0 * d * T * Ts + Ts * Ts);
   filter->a2 = (4.0 * T * T - 4.0 * d * T * Ts + Ts * Ts) / (4.0 * T * T + 4.0 * d * T * Ts + Ts * Ts); 
   filter->b  = 4.0/(4.0 * T * T + 4.0 * d * T * Ts + Ts * Ts);
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
   for (int i = 0; i<filter->signal_dim; i++)
   {
      float u = u_in[i];
      y[i]          =       filter->b * u                   + filter->z1[i];
      filter->z1[i] = 2.0 * filter->b * u - filter->a1 * y[i] + filter->z2[i];
      filter->z2[i] =       filter->b * u - filter->a2 * y[i];
   }
}


void filter_hp_run(Filter2nd *filter, const float *u_in, float *y)
{
   for (int i = 0; i < filter->signal_dim; i++)
   {
      float u = u_in[i];
      y[i]          =  filter->b * u                     + filter->z1[i];
      filter->z1[i] =                - filter->a1 * y[i] + filter->z2[i];
      filter->z2[i] = -filter->b * u - filter->a2 * y[i];
   }
}


void filter_hpd_run(Filter2nd *filter, const float *u_in, float *y)
{
   for (int i = 0; i < filter->signal_dim; i++)
   {
      float u = u_in[i];
      y[i]          =        filter->b * u                     + filter->z1[i];
      filter->z1[i] = -2.0 * filter->b * u - filter->a1 * y[i] + filter->z2[i];
      filter->z2[i] =        filter->b * u - filter->a2 * y[i];
   }
}


float filter_run(Filter2ndFull *filter, const float u)
{
   float y    = filter->b0 * u                  + filter->z1;
   filter->z1 = filter->b1 * u - filter->a1 * y + filter->z2;
   filter->z2 = filter->b2 * u - filter->a2 * y;
   return y;
}

