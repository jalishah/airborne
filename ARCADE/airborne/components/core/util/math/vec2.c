
#include <util.h>
#include <math.h>

#include "vec2.h"


void vec2_set(vec2_t *vo, float x, float y)
{
   vo->x = x;
   vo->y = y;
}


void vec2_scale(vec2_t *vo, const vec2_t *vi, float factor)
{
   FOR_N(i, 2)
   {
      vo->vec[i] = vi->vec[i] * factor;   
   }
}


float vec2_norm(const vec2_t *v)
{
   return sqrtf(v->x * v->x + v->y * v->y);   
}


void vec2_normalize(vec2_t *vo, const vec2_t *vi)
{
   float norm = vec2_norm(vi);
   if (norm != 0.0f)
   {
      FOR_N(i, 2)
      {
         vo->vec[i] = vi->vec[i] / norm;
      }
   }
}


void vec2_add(vec2_t *vo, const vec2_t *vi1, const vec2_t *vi2)
{
   FOR_N(i, 2)
   {
      vo->vec[i] = vi1->vec[i] + vi2->vec[i];
   }
}


void vec2_sub(vec2_t *vo, const vec2_t *vi1, const vec2_t *vi2)
{
   FOR_N(i, 2)
   {
      vo->vec[i] = vi1->vec[i] - vi2->vec[i];
   }
}


void vec2_ortho_right(vec2_t *vo, const vec2_t *vi)
{
   vo->x = vi->y;
   vo->y = -vi->x;
}


float vec2_inner(const vec2_t *v1, const vec2_t *v2)
{
   return v1->x * v2->x + v1->y * v2->y;
}

 
void vec2_project(vec2_t *vo, const vec2_t *vi1, const vec2_t *vi2)
{
   float scale = vec2_inner(vi1, vi2) / vec2_inner(vi2, vi2);
   vec2_scale(vo, vi2, scale);
}

