
#ifndef __RC_DSL_H__
#define __RC_DSL_H__


#include <stdint.h>


struct rc_dsl;
typedef struct rc_dsl rc_dsl_t;


rc_dsl_t *rc_dsl_create(void);


/*
 * parses a single DSL data stream byte
 * returns:
 *    0 if char was processed but no new frame was assembled
 *    1 if a valid frame was assembled
 *   -1 if the frame was invalid
 */
int rc_dsl_parse_dsl_data(rc_dsl_t *dsl, uint8_t b);


void rc_dsl_cmd_show_dsl(rc_dsl_t *dsl);


int rc_dsl_get_channel(rc_dsl_t *dsl, uint8_t channel);

int rc_dsl_get_rssi(rc_dsl_t *dsl);


#endif /* __RC_DSL_H__ */

