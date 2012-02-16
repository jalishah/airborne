
/*
 * holger_fc.h
 *
 * link to holger's flight control
 *
 * Created on: 15.06.2010
 * Author: tobi
 */


#ifndef __HOLGER_FC_H__
#define __HOLGER_FC_H__


#include "../interfaces/altimeter.h"
#include "../interfaces/health.h"
#include "../../platform/mixer.h"


int fc_init(void);


/*
 * reads current altitude
 */
float fc_read_alt(void);


/*
 * reads current battery voltage
 */
int fc_read_voltage(health_data_t *data_out);


/*
 * writes mixer input to motors (and low-level FC controllers)
 */
int fc_write_motors(mixer_in_t *data);


/*
 * spins up motors
 */
void fc_start_motors(void);


/*
 * stops motors
 */
void fc_stop_motors(void);


/*
 * reads out motor RPM
 */
void fc_read_motors_rpm(float *rpm_out);


#endif /* __HOLGER_FC_H__ */

