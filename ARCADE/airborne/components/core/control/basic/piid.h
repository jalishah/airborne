
#ifndef __PIID_H__
#define __PIID_H__


#include "control_param.h"
#include "../util/adams4.h"
#include "../../filters/filter.h"


typedef struct 
{
   float Ts;

   float *f_local;
   float *xi_err;
   float *xii_err;

   adams4_t *int_err1;
   adams4_t *int_err2;

   unsigned int int_enable;

   Filter1 *filter_lp_err;
   Filter1 *filter_hp_err;

   Filter2 *filter_ref;
   Filter2 *filter_feedforw_x;
   Filter2 *filter_feedforw_y;
   Filter2 *filter_feedforw_z;

   float ringbuf[3*CTRL_NUM_TSTEP];
   int ringbuf_idx;

   float test_out[3];
}
piid_t;


int piid_init(piid_t *ctrl, float sample_time);

void piid_run(piid_t *ctrl, float gyro[3], float rc[4], float u_ctrl[3]);

void piid_term(piid_t *ctrl);


#endif /* __PIID_H__ */

