
/*
 * ahrs.c
 *
 * Created on: 11.06.2010
 * Author: tobi
 */


#include "ahrs.h"
#include "../chr_6dm/chr_6dm.h"


int ahrs_init(void)
{
   return chr6dm_init();
}


int ahrs_read(ahrs_data_t *data)
{
   chr6dm_wait_for_data();
   return chr6dm_read(data);   
}

