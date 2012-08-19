
/**
 * vector2d.h
 *
 * purpose: generic 2d vector library interface
 * author: Tobias Simon, Ilmenau University of Technology
 *
 */


#ifndef VECTOR2D_H
#define VECTOR2D_H


typedef struct
{
   void *data;
}
vector2d_t;


void vector2d_alloc(vector2d_t *vector);


void vector2d_set(vector2d_t *out_vec, float x, float y);


void vector2d_copy(vector2d_t *out_vec, vector2d_t *in_vec);


float vector2d_get_x(vector2d_t *in_vec);


float vector2d_get_y(vector2d_t *in_vec);


void vector2d_get_xy(float *x, float *y, vector2d_t *in_vec);


void vector2d_add(vector2d_t *vec_out, vector2d_t *vec_a, vector2d_t *vec_b);


void vector2d_sub(vector2d_t *vec_out, vector2d_t *vec_a, vector2d_t *vec_b);


void vector2d_scalar_multiply(vector2d_t *vec_out, float scale, vector2d_t *vec_in);


float vector2d_length(vector2d_t *vec_in);


void vector2d_normalize(vector2d_t *vec_out, vector2d_t *vec_in);


void vector2d_invert(vector2d_t *vec_out, vector2d_t *vec_in);


float vector2d_dot_product(vector2d_t *vec_a, vector2d_t *vec_b);


void vector2d_orthogonal_right(vector2d_t *vec_out, vector2d_t *vec_in);


void vector2d_project(vector2d_t *vec_out, vector2d_t *vec_a, vector2d_t *vec_b);


#endif /* VECTOR2D_H */

