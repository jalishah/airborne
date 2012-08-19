/*
 * chr-6dm.h
 *
 *  Created on: 30.06.2010
 *      Author: tobi
 */

#ifndef __CHR6DM_H__
#define __CHR6DM_H__


#include "../../interfaces/ahrs.h"


int chr6dm_init(void);

int chr6dm_read(ahrs_data_t *data);

void chr6dm_wait_for_data(void);


#endif /* __CHR6DM_H__ */

