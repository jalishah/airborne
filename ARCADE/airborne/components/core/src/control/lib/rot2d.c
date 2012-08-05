
/**
 * @file rot2d_meschach.c
 * @brief meschach rot2d implementation
 * @author Tobias Simon (Ilmenau University of Technology)
 */


#include <meschach/matrix.h>
#include <math.h>

#include "util.h"
#include "rot2d.h"


struct rot2d_context
{
   MAT *matrix; /**< rotation matrix */
   VEC *in; /**< rotation input vector */
   VEC *out; /**< rotation output vector */
};


void rot2d_calc(rot2d_context_t *context, float out[2], const float in[2], float angle)
{
   ASSERT_NOT_NULL(context->matrix);
   ASSERT_NOT_NULL(context->in);
   ASSERT_NOT_NULL(context->out);

   /*
    * copy arguments to input vector:
    */
   context->in->ve[0] = in[0];
   context->in->ve[1] = in[1];

   /*
    * build rotation matrix:
    *
    * | cos(x) -sin(x) |
    * | sin(x)  cos(x) |
    */
   context->matrix->me[0][0] =  cosf(angle);
   context->matrix->me[0][1] =  sinf(angle);
   context->matrix->me[1][0] = -sinf(angle);
   context->matrix->me[1][1] =  cosf(angle);

   /*
    * perform rotation and result output:
    */
   vm_mlt(context->matrix, context->in, context->out);
   out[0] = context->out->ve[0];
   out[1] = context->out->ve[1];
}


rot2d_context_t *rot2d_create(void)
{
   rot2d_context_t *context = (rot2d_context_t *)malloc(sizeof(rot2d_context_t));
   /* allocate memory for matrix/vector data structures: */
   context->matrix = m_get(2, 2);
   context->in = v_get(2);
   context->out = v_get(2);
   return context;
}

