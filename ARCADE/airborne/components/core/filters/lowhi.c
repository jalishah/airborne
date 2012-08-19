
/*
 * purpose: various filters
 * authors: Alexander Barth; Benjamin Jahn; Tobias Simon, Ilmenau University of Technology
 */


#include <math.h>
#include <malloc.h>
#include "lowhi.h"


void filter_lp_init(filt2nd_t *filt, const float fg, const float d, const float ts, const int signal_dim)
{
   const float T = 1.0 / (2.0 * M_PI * fg);
 
   filt->ts = ts;
   filt->signal_dim = signal_dim;
 
   filt->z1 = malloc(signal_dim * sizeof(float)); 
   filt->z2 = malloc(signal_dim * sizeof(float));

   for (int i = 0; i < signal_dim; i++)
   {
      filt->z1[i] = 0.0;
      filt->z2[i] = 0.0;
   }

   filt->a1 = (2.0 * ts * ts - 8.0 * T * T) / (4.0 * T * T + 4.0 * d * T * ts + ts * ts);
   filt->a2 = (4.0 * T * T - 4.0 * d * T * ts + ts * ts) / (4.0 * T * T + 4.0 * d * T * ts + ts * ts);
   filt->b  = ts * ts / (4.0 * T * T + 4.0 * d * T * ts + ts * ts);
}


void filter_lp_run(filt2nd_t *filt, const float *u_in, float *y)
{
   for (int i = 0; i < filt->signal_dim; i++)
   {
      float u = u_in[i];
      y[i]        =       filt->b * u                   + filt->z1[i];
      filt->z1[i] = 2.0 * filt->b * u - filt->a1 * y[i] + filt->z2[i];
      filt->z2[i] =       filt->b * u - filt->a2 * y[i];
   }
}


void filter_hp_init(filt2nd_t *filt, const float fg, const float d, const float ts, const int signal_dim)
{
   const float T = 1.0 / (2.0 * M_PI * fg);
    
   filt->ts = ts;
   filt->signal_dim = signal_dim;
    
   filt->z1 = malloc(signal_dim * sizeof(float)); 
   filt->z2 = malloc(signal_dim * sizeof(float));

   for (int i = 0; i < signal_dim; i++)
   {
      filt->z1[i] = 0.0;
      filt->z2[i] = 0.0;
   }

   filt->a1 = (2.0 * ts * ts - 8.0 * T * T) / (4.0 * T * T + 4.0 * d * T * ts + ts * ts);
   filt->a2 = (4.0 * T * T - 4.0 * d * T * ts + ts * ts) / (4.0 * T * T + 4.0 * d * T * ts + ts * ts); 
   filt->b  = (2.0 * ts) / (4.0 * T * T + 4.0 * d * T * ts + ts * ts);
}


void filter_hp_run(filt2nd_t *filt, const float *u_in, float *y)
{
   for (int i = 0; i < filt->signal_dim; i++)
   {
      float u = u_in[i];
      y[i]        =  filt->b * u                   + filt->z1[i];
      filt->z1[i] =              - filt->a1 * y[i] + filt->z2[i];
      filt->z2[i] = -filt->b * u - filt->a2 * y[i];
   }
}


void filter_hpd_init(filt2nd_t *filt, const float fg, const float d, const float ts, const int signal_dim)
{
   const float T = 1.0 / (2.0 * M_PI * fg);
    
   filt->ts = ts;
   filt->signal_dim = signal_dim;
    
   filt->z1 = malloc(signal_dim * sizeof(float));
   filt->z2 = malloc(signal_dim * sizeof(float));
    
   int i;
   for (i = 0; i < signal_dim; i++)
   {
      filt->z1[i] = 0;
      filt->z2[i] = 0;
   }
    
   filt->a1 = (2.0 * ts * ts - 8.0 * T * T) / (4.0 * T * T + 4.0 * d * T * ts + ts * ts);
   filt->a2 = (4.0 * T * T - 4.0 * d * T * ts + ts * ts) / (4.0 * T * T + 4.0 * d * T * ts + ts * ts); 
   filt->b  = 4.0/(4.0 * T * T + 4.0 * d * T * ts + ts * ts);
}


void filter_hpd_run(filt2nd_t *filt, const float *u_in, float *y)
{
   for (int i = 0; i < filt->signal_dim; i++)
   {
      float u = u_in[i];
      y[i]        =        filt->b * u                   + filt->z1[i];
      filt->z1[i] = -2.0 * filt->b * u - filt->a1 * y[i] + filt->z2[i];
      filt->z2[i] =        filt->b * u - filt->a2 * y[i];
   }
}


void filter_full_init(filt2nd_full_t *filt, const float *a, const float *b, const float ts)
{
   filt->ts = ts;

   filt->z1 = 0;
   filt->z2 = 0;

   filt->a1 = a[0];
   filt->a2 = a[1];

   filt->b0 = b[0];
   filt->b1 = b[1];
   filt->b2 = b[2];
}


float filter_full_run(filt2nd_full_t *filt, const float u)
{
   float y  = filt->b0 * u                  + filt->z1;
   filt->z1 = filt->b1 * u - filt->a1 * y + filt->z2;
   filt->z2 = filt->b2 * u - filt->a2 * y;
   return y;
}

