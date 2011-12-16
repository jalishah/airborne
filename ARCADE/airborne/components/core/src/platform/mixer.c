
/**
 * file: mixer.c
 * author: Tobias Simon (Ilmenau University of Technology)
 */


#include "mixer.h"
#include <meschach/matrix.h>


struct mixer
{
   size_t motors;
   MAT *matrix;
   VEC *in;
   VEC *out;
};


mixer_t *mixer_create(size_t motors, float *init)
{
   mixer_t *context = malloc(sizeof(mixer_t));

   context->matrix = m_get(motors, 4);
   for (unsigned int i = 0; i < motors; i++)
   {
      for (unsigned int j = 0; j < 4; j++)
      {
         context->matrix->me[i][j] = init[i * 4 + j];
      }
   }

   context->in = v_get(4);
   context->out = v_get(motors);
   context->motors = motors;
   return context;
}


void mixer_calc(mixer_t *context, float *out, const mixer_in_t *in)
{
   for (int i = 0; i < 4; i++)
   {
      context->in->ve[i] = in->v[i];
   }

   mv_mlt(context->matrix, context->in, context->out);

   for (unsigned int i = 0; i < context->motors; i++)
   {
      float val = context->out->ve[i];
      if (val < 0.0f)
      {
         val = 0.0f;
      }
      else if (val > 1.0f)
      {
         val = 1.0f;
      }
      out[i] = val;
   }
}

