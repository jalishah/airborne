

#ifndef __VEC2_H__
#define __VEC2_H__


typedef union
{
   struct
   {
      float x;
      float y;
   };
   float vec[2];
}
vec2_t;


void vec2_set(vec2_t *vo, float x, float y);

void vec2_scale(vec2_t *vo, const vec2_t *vi, float factor);

float vec2_norm(const vec2_t *v);

void vec2_normalize(vec2_t *vo, const vec2_t *vi);

void vec2_add(vec2_t *vo, const vec2_t *vi1, const vec2_t *vi2);

void vec2_sub(vec2_t *vo, const vec2_t *vi1, const vec2_t *vi2);

void vec2_ortho_right(vec2_t *vo, const vec2_t *vi);

float vec2_inner(const vec2_t *v1, const vec2_t *v2);
 
void vec2_project(vec2_t *vo, const vec2_t *vi1, const vec2_t *vi2);


#endif /* __VEC2_H__ */

