
/**
 * @file rot2d.h
 * @brief counterclockwise 2d rotation interface
 * @author Tobias Simon (Ilmenau University of Technology)
 */


#ifndef ROT2D_H
#define ROT2D_H


/**
 * this context has to be initialized by rot2d_impl_t.init
 * after initialization, rot2d_impl_t.calc may be used to calculate the rotation
 */
struct rot2d_context;
typedef struct rot2d_context rot2d_context_t;


rot2d_context_t *rot2d_create(void);

void rot2d_calc(rot2d_context_t *context, float out[2], const float in[2], float angle);


#endif /* ROT2D_H */
