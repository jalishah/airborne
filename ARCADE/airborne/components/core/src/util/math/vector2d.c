
/**
 * vector2d.c
 *
 * purpose: 2d vector library utilizing libmeschach
 * author: Tobias Simon, Ilmenau University of Technology
 *
 */


#include "vector2d.h"

#include <meschach/matrix.h>
#include <math.h>


#define VEC2D_X_INDEX 0
#define VEC2D_Y_INDEX 1


void vector2d_alloc(vector2d_t *vector)
{
   vector->data = v_get(2);
}


void vector2d_set(vector2d_t *out_vec, float x, float y)
{
   VEC *vec = out_vec->data;
   vec->ve[VEC2D_X_INDEX] = x;
   vec->ve[VEC2D_Y_INDEX] = y;
}


void vector2d_copy(vector2d_t *out_vec, vector2d_t *in_vec)
{
   VEC *vout = out_vec->data;
   VEC *vin = in_vec->data;
   vout->ve[VEC2D_X_INDEX] = vin->ve[VEC2D_X_INDEX];
   vout->ve[VEC2D_Y_INDEX] = vin->ve[VEC2D_Y_INDEX];
}


float vector2d_get_x(vector2d_t *in_vec)
{
   return ((VEC *)(in_vec->data))->ve[VEC2D_X_INDEX];
}


float vector2d_get_y(vector2d_t *in_vec)
{
   return ((VEC *)(in_vec->data))->ve[VEC2D_Y_INDEX];
}


void vector2d_get_xy(float *x, float *y, vector2d_t *in_vec)
{
   *x = vector2d_get_x(in_vec);
   *y = vector2d_get_y(in_vec);
}


void vector2d_add(vector2d_t *vec_out, vector2d_t *vec_a, vector2d_t *vec_b)
{
   v_add(vec_a->data, vec_b->data, vec_out->data);
}


void vector2d_sub(vector2d_t *vec_out, vector2d_t *vec_a, vector2d_t *vec_b)
{
   v_sub(vec_a->data, vec_b->data, vec_out->data);
}


void vector2d_scalar_multiply(vector2d_t *vec_out, float scale, vector2d_t *vec_in)
{
   sv_mlt(scale, vec_in->data, vec_out->data);
}


float vector2d_length(vector2d_t *vec_in)
{
   return v_norm2(vec_in->data);
}


void vector2d_normalize(vector2d_t *vec_out, vector2d_t *vec_in)
{
   float factor;
   if (vector2d_length(vec_in) == 0.0)
   {
      factor = 0.0;
   }
   else
   {
      factor = 1.0 / vector2d_length(vec_in);
   }
   vector2d_scalar_multiply(vec_out, factor, vec_in);
}


void vector2d_invert(vector2d_t *vec_out, vector2d_t *vec_in)
{
   vector2d_scalar_multiply(vec_out, -1.0, vec_in);
}


float vector2d_dot_product(vector2d_t *vec_a, vector2d_t *vec_b)
{
   return in_prod(vec_a->data, vec_b->data);
}


void vector2d_orthogonal_right(vector2d_t *vec_out, vector2d_t *vec_in)
{
   VEC *mv_in = vec_in->data;
   VEC *mv_out = vec_out->data;
   mv_out->ve[VEC2D_X_INDEX] = mv_in->ve[VEC2D_Y_INDEX];
   mv_out->ve[VEC2D_Y_INDEX] = -mv_in->ve[VEC2D_X_INDEX];
}


/*
 * project vector a on direction vector b
 */
void vector2d_project(vector2d_t *vec_out, vector2d_t *vec_a, vector2d_t *vec_b)
{
   vector2d_scalar_multiply(vec_out, vector2d_dot_product(vec_a, vec_b) / vector2d_dot_product(vec_b, vec_b), vec_b);
}

