
/*
 * file: coupling.c
 * author: Tobias Simon, Ilmenau University of Technology
 */


#include <errno.h>

#include "coupling.h"


void coupling_init(coupling_t *coupling, const size_t n_motors, const float *init)
{
   coupling->matrix = m_get(n_motors, 4);
   for (int i = 0; i < n_motors; i++)
   {
      for (int j = 0; j < 4; j++)
      {
         coupling->matrix->me[i][j] = init[i * 4 + j];
      }
   }
   coupling->in = v_get(4);
   coupling->out = v_get(n_motors);
   coupling->n_motors = n_motors;
}


void coupling_calc(const coupling_t *coupling, float *out, const float *in)
{
   /* copy data into input vector: */
   for (int i = 0; i < 4; i++)
   {
      coupling->in->ve[i] = in[i];
   }

   /* matrix-vector multiplication: */
   mv_mlt(coupling->matrix, coupling->in, coupling->out);

   /* copy result of computation into output vector: */
   for (size_t i = 0; i < coupling->n_motors; i++)
   {
      out[i] = coupling->out->ve[i];
   }
}

