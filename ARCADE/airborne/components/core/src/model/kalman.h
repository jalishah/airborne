
/*
 * file: kalman.h
 *
 * author: Tobias Simon, Ilmenau University of Technology
 */


#ifndef __KALMAN_H__
#define __KALMAN_H__


typedef struct
{
   float pos;
   float speed;
}
kalman_out_t;


typedef struct
{
   float dt; /* delta t in sec */
   float pos; /* position in m */
   float acc; /* acceleration min m/sec */
}
kalman_in_t;


typedef struct
{
   float process_var;
   float measurement_var;
}
kalman_config_t;


struct kalman;
typedef struct kalman kalman_t;


void kalman_run(kalman_out_t *out, kalman_t *kalman, const kalman_in_t *in);
kalman_t *kalman_create(const kalman_config_t *config, const kalman_out_t *init_state);
void kalman_free(kalman_t *kalman);


#endif /* __KALMAN_H__ */
