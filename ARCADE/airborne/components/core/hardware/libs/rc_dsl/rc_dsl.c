
#include "rc_dsl.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


#define RC_DSL_VALID_SIGNAL 7
#define RC_MAX_CHANNEL      16


struct rc_dsl
{
   /* raw bytes: */
   uint8_t raw_data[50];

   /* meta data: */
   uint8_t RSSI;       /* signal strength indicator */
   uint8_t battery;    /* battery level */
   uint8_t allocation; /* band allocation (35,40,72) */
   uint8_t channel;    /* receiver channel */

   /* rc channels: */
   int16_t rc_ch[RC_MAX_CHANNEL];

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
};


rc_dsl_t *rc_dsl_create(void)
{
   rc_dsl_t *dsl = malloc(sizeof(rc_dsl_t));
   memset(dsl, 0, sizeof(rc_dsl_t));
   return dsl;
}


/*
 * new rc signal received
 *
 * this function is called, when a new servo signal is properly received.
 * parameters: servo  - servo number (0-9)
 *             signal - servo signal between 7373 (1ms) and 14745 (2ms)
 */
static void rc_dsl_new_dsl_rcsignal(rc_dsl_t *dsl, uint8_t servo, int16_t signal)
{
   /* scale signal: */
   signal -= 11059; /* shift neutral position to 0 */
   signal /= 6; /* scale 10bit [-512..512] */
   signal *= 4; /* scale 12bit [-2048..2048] */

   /* limit signal: */
   if (signal > 2048)
   {
      signal = 2048;
   }
   if (signal < -2048)
   {
      signal = -2048;
   }

   /* store signal: */
   dsl->rc_ch[servo] = signal;
}


/*
 * parse a real DSL packet
 *
 * this function is called within rc_dsl_parse_data(), when a complete
 * data paket with matching checksum has been received.
 */
static int rc_dsl_parse_incoming_dsl_paket(rc_dsl_t *dsl)
{
   if (dsl->raw_data[0] == 0x1F)
   {
      /* process status header: */
      dsl->status_packets++;
      dsl->allocation = dsl->raw_data[0 + 1];
      dsl->channel = dsl->raw_data[1 + 1];
      dsl->RSSI = dsl->raw_data[2 + 1];
      dsl->battery = dsl->raw_data[3 + 1];
      return 0;
   }
   else if ((dsl->raw_data[0] & 0xF0) == 0x10)
   {
      /* process signal header: */
      uint8_t i;
      dsl->signal_packets++;

      /* last 4 bits of the header indicates servo pair: */
      i = dsl->raw_data[0] & 0x0F;

      if (i < 10)
      {
         typedef union
         {
            uint16_t pos[2];
            uint8_t dat[4];
         }
         rc_dsl_servos_t;

         rc_dsl_servos_t servos;

         /* convert byte array to two uint16: */
         servos.dat[1] = dsl->raw_data[1];
         servos.dat[0] = dsl->raw_data[2];
         servos.dat[3] = dsl->raw_data[3];
         servos.dat[2] = dsl->raw_data[4];

         /* store new servo data: */
         rc_dsl_new_dsl_rcsignal(dsl, i, (int16_t) servos.pos[0]);
         rc_dsl_new_dsl_rcsignal(dsl, i + 1, (int16_t) servos.pos[1]);
      }
      return 0;
   }

   /* unknown packet type */
   dsl->packet_unknown++;
   return -1;
}


/*
 * parses a single DSL data stream byte
 * returns:
 *    0 if char was processed but no new frame was assembled
 *    1 if a valid frame was assembled
 *   -1 if the frame was invalid
 */
int rc_dsl_parse_dsl_data(rc_dsl_t *dsl, uint8_t b)
{
   int status = 0;

   /* check for sync condition */
   if ((b == 0xFF) && (dsl->last_byte == 0xFF))
   {
      dsl->data_counter = 0;
      dsl->check_sum = 0;
      goto out;
   }

   /* first byte is cmd */
   if (dsl->data_counter == 0)
   {
      if (b == 0x1F)
      {
         dsl->packet_len = 5;
      }
      else if ((b & 0xF0) == 0x10)
      {
         dsl->packet_len = 4;
      }
      else if (b == 0xE3)
      {
         dsl->packet_len = 48;
      }
   }

   /* last byte is checksum */
   if ((b != 0xFF) && (dsl->data_counter > dsl->packet_len))
   {
      /* calculate checksum */
      dsl->check_sum = ~(dsl->check_sum);
      if (dsl->check_sum == 0xFF)
      {
         dsl->check_sum = 0xFE;
      }

      if (b == dsl->check_sum)
      {
         status = rc_dsl_parse_incoming_dsl_paket(dsl);
      }
      else
      {
         status = -1;
         dsl->packet_invalid++;
      }

      /* prepare for a new data paket */
      dsl->data_counter = 0;
      dsl->check_sum = 0;
   }
   else
   {
      /* new byte within a paket */
      dsl->raw_data[dsl->data_counter++] = b;
      dsl->check_sum += b;
   }

   /* always remember last byte received for
      detection of sync condition */
   dsl->last_byte = b;

out:
   return status;
}


void rc_dsl_cmd_show_dsl(rc_dsl_t *dsl)
{
   printf("signal quality: \t%d\n", dsl->RSSI);
   printf("battery:        \t%d\n", dsl->battery);
   printf("band:           \t%d\n", dsl->allocation);
   printf("channel:        \t%d\n", dsl->channel);
   printf("invalid packets:\t%d\n", (int)dsl->packet_invalid);
   printf("unknown packets:\t%d\n", (int)dsl->packet_unknown);
   printf("status packets: \t%d\n", (int)dsl->status_packets);
   printf("signal packets: \t%d\n", (int)dsl->signal_packets);
   printf("scan packets:   \t%d\n", (int)dsl->scan_packets);

   printf("\nchannels:\n");
   int c;
   for (c = 0; c < RC_MAX_CHANNEL; c++)
   {
      printf("-C%02d- ", c);
   }
   printf("\n");

   if (dsl->RSSI > RC_DSL_VALID_SIGNAL)
   {
      for (c = 0; c < RC_MAX_CHANNEL; c++)
      {
         printf("%+04d  ", (int)dsl->rc_ch[c]);
      }
   }
   else
   {
      for (c = 0; c < RC_MAX_CHANNEL; c++)
      {
         printf(" N/A  ");
      }
   }
   printf("\n");
}


int rc_dsl_get_channel(rc_dsl_t *dsl, uint8_t channel)
{
   assert(channel < RC_MAX_CHANNEL);
   return dsl->rc_ch[channel];
}


int rc_dsl_signal_valid(rc_dsl_t *dsl)
{
   return dsl->RSSI > RC_DSL_VALID_SIGNAL;
}


int rc_dsl_invalid(rc_dsl_t *dsl)
{
   return dsl->packet_invalid;
}

