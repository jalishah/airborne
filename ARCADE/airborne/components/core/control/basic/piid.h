
#ifndef __PIID_H__
#define __PIID_H__


#include <stdbool.h>

#include "control_param.h"
#include "../util/adams4.h"
#include "../../filters/lowhi.h"


typedef struct 
{
   float ts;
    
   float *f_local;
   float *xi_err;
   float *xii_err;
    
   adams4_t *int_err1;
   adams4_t *int_err2;
 
   bool int_enable;
 
   filt2nd_t *filter_lp_err;
   filt2nd_t *filter_hp_err;
   filt2nd_full_t *filter_feedforw_x;
   filt2nd_full_t *filter_feedforw_y;
   filt2nd_full_t *filter_feedforw_z;
    
   float ringbuf[3 * CTRL_NUM_TSTEP];
   int ringbuf_idx;
}
piid_t;


int piid_init(piid_t *ctrl, const float sample_time);

void piid_run(piid_t *ctrl, const float gyro[3], const float rc[3], float u_ctrl[3]);

void piid_term(piid_t *ctrl);


#endif /* __PIID_H__ */

