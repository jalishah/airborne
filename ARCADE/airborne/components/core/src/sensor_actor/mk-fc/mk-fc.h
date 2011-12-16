/*
 * mk-fc.h
 *
 * Created on: 15.06.2010
 * Author: tobi
 */


#ifndef __MK_FC_H__
#define __MK_FC_H__


#include "../interfaces/altimeter.h"
#include "../interfaces/health.h"
#include "../../platform/mixer.h"


int fc_init(void);

float fc_read_alt(void);

float fc_read_voltage(health_data_t *data_out);

int fc_write_motors(mixer_in_t *data);

void fc_start_motors(void);

void fc_stop_motors(void);

void fc_read_motors_rpm(float *rpm_out);


#endif /* __MK_FC_H__ */

