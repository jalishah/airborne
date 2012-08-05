
/*
 * file: coupling.c
 * author: Tobias Simon (Ilmenau University of Technology)
 */


#include "coupling.h"
#include <meschach/matrix.h>


struct coupling
{
   size_t motors;
   MAT *matrix;
   VEC *in;
   VEC *out;
};


coupling_t *coupling_create(size_t motors, float *init)
{
   coupling_t *coupling = malloc(sizeof(coupling_t));
   coupling->matrix = m_get(motors, 4);
   for (int i = 0; i < motors; i++)
   {
      for (int j = 0; j < 4; j++)
      {
         coupling->matrix->me[i][j] = init[i * 4 + j];
      }
   }
   coupling->in = v_get(4);
   coupling->out = v_get(motors);
   coupling->motors = motors;
   return coupling;
}


void coupling_calc(coupling_t *coupling, float *out, float *in)
{
   /* copy data into input vector: */
   for (int i = 0; i < 4; i++)
   {
      coupling->in->ve[i] = in[i];
   }

   /* matrix-vector multiplication: */
   mv_mlt(coupling->matrix, coupling->in, coupling->out);

   /* copy result of computation into output vector: */
   for (int i = 0; i < coupling->motors; i++)
   {
      out[i] = coupling->out->ve[i];
   }
}


