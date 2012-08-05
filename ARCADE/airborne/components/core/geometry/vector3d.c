
/**
 * vector3d.c
 *
 * purpose: 3d vector library utilizing libmeschach
 * author: Tobias Simon, Ilmenau University of Technology
 *
 */


#include "vector3d.h"

#include <meschach/matrix.h>
#include <math.h>


#define VEC3D_X_INDEX 0
#define VEC3D_Y_INDEX 1
#define VEC3D_Z_INDEX 2


void vector3d_alloc(vector3d_t *vector)
{
   vector->data = v_get(3);
}


void vector3d_set(vector3d_t *out_vec, float x, float y, float z)
{
   VEC *vec = out_vec->data;
   vec->ve[VEC3D_X_INDEX] = x;
   vec->ve[VEC3D_Y_INDEX] = y;
   vec->ve[VEC3D_Z_INDEX] = z;
}


float vector3d_get_x(vector3d_t *in_vec)
{
   return ((VEC *)(in_vec->data))->ve[VEC3D_X_INDEX];
}


float vector3d_get_y(vector3d_t *in_vec)
{
   return ((VEC *)(in_vec->data))->ve[VEC3D_Y_INDEX];
}

float vector3d_get_z(vector3d_t *in_vec)
{
   return ((VEC *)(in_vec->data))->ve[VEC3D_Z_INDEX];
}


void vector3d_get_xy(float *x, float *y, float *z, vector3d_t *in_vec)
{
   *x = vector3d_get_x(in_vec);
   *y = vector3d_get_y(in_vec);
   *z = vector3d_get_z(in_vec);
}


void vector3d_add(vector3d_t *vec_out, vector3d_t *vec_a, vector3d_t *vec_b)
{
   v_add(vec_a->data, vec_b->data, vec_out->data);
}


void vector3d_sub(vector3d_t *vec_out, vector3d_t *vec_a, vector3d_t *vec_b)
{
   v_sub(vec_a->data, vec_b->data, vec_out->data);
}


void vector3d_scalar_multiply(vector3d_t *vec_out, float scale, vector3d_t *vec_in)
{
   sv_mlt(scale, vec_in->data, vec_out->data);
}


float vector3d_length(vector3d_t *vec_in)
{
   return v_norm2(vec_in->data);
}


void vector3d_normalize(vector3d_t *vec_out, vector3d_t *vec_in)
{
   float factor;
   if (vector3d_length(vec_in) == 0.0)
   {
      factor = 0.0;
   }
   else
   {
      factor = 1.0 / vector3d_length(vec_in);
   }
   vector3d_scalar_multiply(vec_out, factor, vec_in);
}


void vector3d_invert(vector3d_t *vec_out, vector3d_t *vec_in)
{
   vector3d_scalar_multiply(vec_out, -1.0, vec_in);
}


float vector3d_dot_product(vector3d_t *vec_a, vector3d_t *vec_b)
{
   return in_prod(vec_a->data, vec_b->data);
}

void vector3d_project(vector3d_t *vec_out, vector3d_t *vec_a, vector3d_t *vec_b)
{
   vector3d_scalar_multiply(vec_out, vector3d_dot_product(vec_a, vec_b) / vector3d_dot_product(vec_b, vec_b), vec_b);
}

