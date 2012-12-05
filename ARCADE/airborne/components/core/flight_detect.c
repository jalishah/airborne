

#include "filters/sliding_var.h"
#include "flight_detect.h"

#include <util.h>
#include <malloc.h>


static sliding_var_t *vars;
static size_t dim;
static size_t init_cnt;
static size_t wnd;
static float tresh;
static float *wghts;


void flight_detect_init(size_t dimension, size_t window, float treshold, float *weights)
{
   ASSERT_ONCE();
   wghts = weights;
   tresh = treshold;
   wnd = window;
   dim = dimension;
   vars = malloc(dim * sizeof(sliding_var_t));
   ASSERT_NOT_NULL(vars);
   FOR_N(i, dim)
   {
      sliding_var_init(&vars[i], window, 0.0f);   
   }
}


int flight_detect(float *in)
{
   init_cnt++;
   if (init_cnt < wnd)
   {
      return 0;
   }
   init_cnt = wnd;
   float sum = 0.0f;
   FOR_N(i, dim)
   {
      if (wghts)
         sum += wghts[i] * sliding_var_calc(&vars[i], in[i]);
      else
         sum += sliding_var_calc(&vars[i], in[i]);
   }
   printf("%f\n", sum);
   return sum > tresh;
}

