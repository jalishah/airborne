

#include "../filters/sliding_var.h"
#include "flight_detect.h"

#include <util.h>
#include <malloc.h>


static sliding_var_t *vars;
static size_t dim;
static size_t init_cnt;
static size_t wnd;
static float fly_tresh;
static float crash_tresh;
static float hyst;
static int hyst_cnt = 0;
static flight_state_t state = FS_STANDING;


void flight_detect_init(size_t window, size_t hysteresis, float fly_treshold, float crash_treshold)
{
   ASSERT_ONCE();
   hyst = hysteresis;
   fly_tresh = fly_treshold;
   crash_tresh = crash_treshold;
   wnd = window;
   dim = 3;
   vars = malloc(dim * sizeof(sliding_var_t));
   ASSERT_NOT_NULL(vars);
   FOR_N(i, dim)
   {
      sliding_var_init(&vars[i], window, 0.0f);   
   }
}


flight_state_t flight_detect(float acc[3])
{
   /* perfom signal processing: */
   float sum = 0;
   FOR_N(i, dim)
   {
      float var = sliding_var_calc(&vars[i], acc[i]);
      sum += var;
   }
   sum /= dim;

   /* signal/hysteresis-based state identification: */
   
   if (sum > crash_tresh)
      state = FS_CRASHED;

   if (state != FS_CRASHED)
   {
      if (sum > fly_tresh)
      {
         /* flying -> flying / standing -> flying */
         state = 1;
         hyst_cnt = 0;
      }
      else
      {
         if (state == 1)
         {
            /* flying -> standing */
            if (hyst_cnt++ == hyst)
            {
               state = 0;
            }
         }
      }
   }
   return state;
}

