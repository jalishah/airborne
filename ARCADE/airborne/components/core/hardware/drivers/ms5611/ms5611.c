
/*
   MS5611 I2C Linux Userspace Driver

   Copyright (C) 2012 Tobias Simon

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
*/


#include <errno.h>
#include <stdint.h>
#include <math.h>

#include <util.h>

#include "ms5611.h"


#define MS5611_ADDRESS      0x77
#define MS5611_ADC          0x00
#define MS5611_RESET        0x1E
#define MS5611_CONV_D1(osr) (0x40 | ((osr) << 1))
#define MS5611_CONV_D2(osr) (0x50 | ((osr) << 1))
#define MS5611_PROM_READ(x) (0xA0 | ((x) << 1))



static const int conv_time_ms[5] =
{
   1 /* 0.60ms */,
   2 /* 1.17 */,
   3 /* 2.28 */,
   5 /* 4.54 */,
   10 /* 9.04 */
};


/* reads prom register into val and returns 0
 * if the read failed, val remains untouched
 * and a negative error code is returned */
static int ms5611_read_prom(ms5611_t *ms5611, uint8_t reg)
{
   THROW_BEGIN();
   uint8_t raw[2];
   THROW_ON_ERR(i2c_read_block_reg(&ms5611->i2c_dev, MS5611_PROM_READ(reg), raw, sizeof(raw)));
   ms5611->prom[reg] = raw[1] | (raw[0] << 8);
   THROW_END();
}


static int ms5611_read_adc(uint32_t *val, ms5611_t *ms5611)
{
   THROW_BEGIN();
   uint8_t raw[3]; /* 24-bit adc data */
   THROW_ON_ERR(i2c_read_block_reg(&ms5611->i2c_dev, MS5611_ADC, raw, sizeof(raw)));
   *val = raw[2] | (raw[1] << 8) | (raw[0] << 16);
   THROW_END();
}


/* starts temperature conversion using configurable oversampling rate */
static int ms5611_start_temp_conv(ms5611_t *ms5611)
{
   THROW_PROPAGATE(i2c_write(&ms5611->i2c_dev, MS5611_CONV_D2(ms5611->t_osr)));
}


/* starts pressure conversion using configurable oversampling rate */
static int ms5611_start_pressure_conv(ms5611_t *ms5611)
{
   THROW_PROPAGATE(i2c_write(&ms5611->i2c_dev, MS5611_CONV_D1(ms5611->p_osr)));
}


/* resets the device */
static int ms5611_reset(ms5611_t *ms5611)
{
   THROW_PROPAGATE(i2c_write(&ms5611->i2c_dev, MS5611_RESET));
}


static uint16_t ms5611_crc4(uint16_t *n_prom)
{
   uint16_t n_rem = 0; /* crc remainder */
   uint16_t crc_read; /* original value of the crc */
   crc_read = n_prom[7]; /* save read CRC */
   n_prom[7] = (0xFF00 & (n_prom[7])); /* CRC byte is replaced by 0 */
   int cnt;
   for (cnt = 0; cnt < 16; cnt++)
   {
      if (cnt % 2 == 1)
      {
         n_rem ^= (n_prom[cnt>>1]) & 0x00FF;
      }
      else
      {
         n_rem ^= n_prom[cnt>>1] >> 8;
      }
      int n_bit;
      for (n_bit = 8; n_bit > 0; n_bit--)
      {
         if (n_rem & (0x8000))
         {
            n_rem = (n_rem << 1) ^ 0x3000;
         }
         else 
         {
            n_rem = (n_rem << 1);
         }
      }
   }
   n_rem = (n_rem >> 12) & 0xF; /* final 4-bit reminder is CRC code */
   n_prom[7] = crc_read;
   return n_rem;
}


int ms5611_init(ms5611_t *ms5611, i2c_bus_t *bus, ms5611_osr_t p_osr, ms5611_osr_t t_osr)
{
   THROW_BEGIN();
   /* copy values */
   i2c_dev_init(&ms5611->i2c_dev, bus, MS5611_ADDRESS);

   /* assign over-sampling settings: */
   ms5611->p_osr = p_osr;
   ms5611->t_osr = t_osr;

   /* reset device: */
   THROW_ON_ERR(ms5611_reset(ms5611));
   msleep(3); /* at least 2.8ms */
   
   /* read prom including CRC */
   int i;
   for (i = 0; i < 8; i++)
   {
      THROW_ON_ERR(ms5611_read_prom(ms5611, i));
   }
   
   /* validate CRC: */
   uint16_t crc = ms5611_crc4(ms5611->prom);
   uint16_t crcProm = ms5611->prom[7] & 0x0F;
   THROW_IF(crc != crcProm, -ENODEV);
   THROW_END();
}


static void ms5611_compensate(ms5611_t *ms5611)
{
   /* aliases for data sheet conformity including casts: */
   int64_t C1 = ms5611->prom[1];
   int64_t C2 = ms5611->prom[2];
   int64_t C3 = ms5611->prom[3];
   int64_t C4 = ms5611->prom[4];
   int64_t C5 = ms5611->prom[5];
   int64_t C6 = ms5611->prom[6];
   int64_t D1 = ms5611->raw_p;
   int64_t D2 = ms5611->raw_t;
   
   /* calculate temperature: */
   int64_t dT = D2 - (C5 << 8); /* reference temperature delta */
   int64_t TEMP = 2000 + ((dT * C6) >> 23);
  
   /* compute temperature compensation: */
   int64_t OFF = (C2 << 16) + ((C4 * dT) >> 7);
   int64_t SENS = (C1 << 15) + ((C3 * dT) >> 8);
   
   /* compute 2nd order temperature compensation, if required: */
   if (TEMP < 2000)
   {
      /* low temperature: */
      int64_t T2 = (dT * dT) >> 31;
      int64_t tmp = (TEMP - 2000) * (TEMP - 2000);
      int64_t OFF2 = 5 * tmp >> 1;
      int64_t SENS2 = 5 * tmp >> 2;
      if (TEMP < -1500)
      {
         /* very low temperature: */
         tmp = (TEMP + 1500) * (TEMP + 1500);
         OFF2 = OFF2 + 7 * tmp;
         SENS2 = SENS2 + (11 * tmp >> 1);
      }
      TEMP = TEMP - T2;
      OFF = OFF - OFF2;
      SENS = SENS - SENS2;
   }

   /* compute compensated pressure: */
   ms5611->c_p = (((D1 * SENS) >> 21) - OFF) >> 15;
   ms5611->c_a = (44330.0 * (1.0 - pow((float)ms5611->c_p / 101325.0, 0.190295)));
   ms5611->c_t = (float)TEMP / 100.0;
}


int ms5611_measure(ms5611_t *ms5611)
{
   THROW_BEGIN();

   /* read temperature: */
   THROW_ON_ERR(ms5611_start_temp_conv(ms5611));
   msleep(conv_time_ms[ms5611->t_osr]); /* 1ms safety */
   THROW_ON_ERR(ms5611_read_adc(&ms5611->raw_t, ms5611));

   /* read pressure: */
   THROW_ON_ERR(ms5611_start_pressure_conv(ms5611));
   msleep(conv_time_ms[ms5611->p_osr]); /* 1ms safety */
   THROW_ON_ERR(ms5611_read_adc(&ms5611->raw_p, ms5611));

   ms5611_compensate(ms5611);

   THROW_END();
}



