
/*
 * file: mk_blmc.c
 * purpose: mikrokopter brushless motor controller interface
 * author: Tobias Simon
 */


#ifndef __MK_BLMC_H__
#define __MK_BLMC_H__


#include <stdint.h>
#include <stddef.h>


struct mk_blmc;
typedef struct mk_blmc mk_blmc_t;


mk_blmc_t *mk_blmc_create(uint8_t adapter, uint8_t *addr_table, size_t n_motors);


void mk_blmc_write(mk_blmc_t *motors, float *setpoints);
void mk_blmc_write_uint8(mk_blmc_t *motors, char *setpoints);


#endif /* __MK_MOTORS_H__ */

