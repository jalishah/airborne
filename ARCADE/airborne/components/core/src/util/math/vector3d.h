
/**
 * vector3d.h
 *
 * purpose: generic 3d vector library interface
 * author: Tobias Simon, Ilmenau University of Technology
 *
 */


#ifndef VECTOR3D_H
#define VECTOR3D_H


typedef struct
{
   void *data;
}
vector3d_t;


void vector3d_alloc(vector3d_t *vector);


void vector3d_set(vector3d_t *out_vec, float x, float y, float z);


float vector3d_get_x(vector3d_t *in_vec);


float vector3d_get_y(vector3d_t *in_vec);


float vector3d_get_z(vector3d_t *in_vec);


void vector3d_get_xy(float *x, float *y, float *z, vector3d_t *in_vec);


void vector3d_add(vector3d_t *vec_out, vector3d_t *vec_a, vector3d_t *vec_b);


void vector3d_sub(vector3d_t *vec_out, vector3d_t *vec_a, vector3d_t *vec_b);


void vector3d_scalar_multiply(vector3d_t *vec_out, float scale, vector3d_t *vec_in);


float vector3d_length(vector3d_t *vec_in);


void vector3d_normalize(vector3d_t *vec_out, vector3d_t *vec_in);


void vector3d_invert(vector3d_t *vec_out, vector3d_t *vec_in);


float vector3d_dot_product(vector3d_t *vec_a, vector3d_t *vec_b);


void vector3d_project(vector3d_t *vec_out, vector3d_t *vec_a, vector3d_t *vec_b);


#endif /* VECTOR3D_H */

