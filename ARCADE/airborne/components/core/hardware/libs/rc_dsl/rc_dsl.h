
#ifndef __RC_DSL_H__
#define __RC_DSL_H__


#include <stdint.h>


#define RC_DSL_VALID_SIGNAL  7
#define RC_DSL_CHANNELS     16

#define RSSI_SCALE(x) (((float)x) / 255.0f)


typedef struct
{
   /* raw bytes: */
   uint8_t raw_data[50];

   /* meta data: */
   uint8_t battery;    /* battery level */
   uint8_t allocation; /* band allocation (35,40,72) */
   uint8_t channel;    /* receiver channel */

   /* RSSI and RC channels: */
   float RSSI;
   float channels[RC_DSL_CHANNELS];

   /* parser state information: */
   uint8_t data_counter;
   uint8_t last_byte;
   uint8_t check_sum;
   uint8_t packet_len;

   enum
   {
      RC_DSL_CMD_NONE = 0,     /* no command running, parsing packets */
      RC_DSL_CMD_RUNNING = 1,  /* command running (eg. scan channels), parsing packets */
      RC_DSL_CMD_FINISHED = 2, /* command finished, not parsing packets until data is consumed */
   }
   cmd_status; /* control packet parsing for command execution (eg. scan channels) */

   /* statistics: */
   uint32_t packet_invalid; /* number of invalid DSL packets received */
   uint32_t packet_unknown; /* number of unknown DSL packets received */
   uint32_t status_packets; /* number of status packets received */
   uint32_t signal_packets; /* number of signal packets received */
   uint32_t scan_packets;   /* number of scan packets received */
}
rc_dsl_t;



void rc_dsl_init(rc_dsl_t *dsl);


/*
 * parses a single DSL data stream byte
 * returns:
 *    0 if char was processed but no new frame was assembled
 *    1 if a valid frame was assembled
 *   -1 if the frame was invalid
 */
int rc_dsl_parse_dsl_data(rc_dsl_t *dsl, uint8_t b);


#endif /* __RC_DSL_H__ */

