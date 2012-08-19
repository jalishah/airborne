
/*
 * bmp085.c
 *
 * created on: 11.06.2010
 * author: tobi
 */


#include "bmp085.h"
#include "util.h"
#include "../../../util/time/ltime.h"



#define BMP085_CAL_DATA_START  0xAA
#define BMP085_CAL_DATA_LENGTH 22



/*
 * reads calibration data from dev and stores it in ctx
 */
static int bmp085_read_calibration_values(i2c_dev_t *dev, bmp085_ctx_t *ctx);

/*
 * reads temperature from dev and stores it in ctx
 */
static int bmp085_update_raw_temperature(i2c_dev_t *dev, bmp085_ctx_t *ctx);

/*
 * reads pressure from dev and stores it in ctx
 */
static int bmp085_update_raw_pressure(i2c_dev_t *dev, bmp085_ctx_t *ctx);


int bmp085_init(i2c_dev_t *dev, bmp085_ctx_t *ctx)
{
   ASSERT_NOT_NULL(dev);
   ASSERT_NOT_NULL(ctx);

   ctx->oversampling_setting = 0x03;
   return bmp085_read_calibration_values(dev, ctx);
}


int bmp085_read_temperature(i2c_dev_t *dev, bmp085_ctx_t *ctx)
{
   ASSERT_NOT_NULL(dev);
   ASSERT_NOT_NULL(ctx);

   int status = bmp085_update_raw_temperature(dev, ctx);
   if (status < 0)
   {
      return status;
   }

   bmp085_cal_t *cal = &ctx->calibration;
   long x1 = ((ctx->raw_temperature - cal->AC6) * cal->AC5) >> 15;
   long x2 = (cal->MC << 11) / (x1 + cal->MD);
   ctx->b6 = x1 + x2 - 4000;

   return (x1 + x2 + 8) >> 4;
}


int bmp085_read_pressure(i2c_dev_t *dev, bmp085_ctx_t *ctx)
{
   ASSERT_NOT_NULL(dev);
   ASSERT_NOT_NULL(ctx);

   bmp085_cal_t *cal = &ctx->calibration;
   long x1, x2, x3, b3;
   unsigned long b4, b7;
   long p;

   bmp085_update_raw_pressure(dev, ctx);

   x1 = (ctx->b6 * ctx->b6) >> 12;
   x1 *= cal->B2;
   x1 >>= 11;

   x2 = cal->AC2 * ctx->b6;
   x2 >>= 11;
   x3 = x1 + x2;

   b3 = (((((long)cal->AC1) * 4 + x3) << ctx->oversampling_setting) + 2) >> 2;

   x1 = (cal->AC3 * ctx->b6) >> 13;
   x2 = (cal->B1 * ((ctx->b6 * ctx->b6) >> 12)) >> 16;
   x3 = (x1 + x2 + 2) >> 2;
   b4 = (cal->AC4 * (unsigned long)(x3 + 32768)) >> 15;

   b7 = ((unsigned long)ctx->raw_pressure - b3) * (50000 >> ctx->oversampling_setting);
   p = ((b7 < 0x80000000) ? ((b7 << 1) / b4) : ((b7 / b4) * 2));

   x1 = p >> 8;
   x1 *= x1;
   x1 = (x1 * 3038) >> 16;
   x2 = (-7357 * p) >> 16;
   p += (x1 + x2 + 3791) >> 4;

   return p;
}


static int bmp085_read_calibration_values(i2c_dev_t *dev, bmp085_ctx_t *ctx)
{
   ASSERT_NOT_NULL(dev);
   ASSERT_NOT_NULL(ctx);

   uint8_t data[BMP085_CAL_DATA_LENGTH];

   i2c_dev_lock_bus(dev);

   for (int i = 0; i < BMP085_CAL_DATA_LENGTH; i++)
   {
      int status = i2c_dev_read(dev, BMP085_CAL_DATA_START + i);
      if (status < 0)
      {
         i2c_dev_unlock_bus(dev);
         return status;
      }
      data[i] = (uint8_t)status;
   }

   i2c_dev_unlock_bus(dev);

   bmp085_cal_t *cal = &ctx->calibration;

   cal->AC1 = (data[0]  << 8) | data[1];
   cal->AC2 = (data[2]  << 8) | data[3];
   cal->AC3 = (data[4]  << 8) | data[5];
   cal->AC4 = (data[6]  << 8) | data[7];
   cal->AC5 = (data[8]  << 8) | data[9];
   cal->AC6 = (data[10] << 8) | data[11];

   cal->B1 = (data[12] << 8) | data[13];
   cal->B2 = (data[14] << 8) | data[15];

   cal->MB = (data[16] << 8) | data[17];
   cal->MC = (data[18] << 8) | data[19];
   cal->MD = (data[20] << 8) | data[21];

   return 0;
}


static int bmp085_update_raw_pressure(i2c_dev_t *dev, bmp085_ctx_t *ctx)
{
   ASSERT_NOT_NULL(dev);
   ASSERT_NOT_NULL(ctx);

   i2c_dev_lock_bus(dev);

   /* start pressure conversion: */
   int status = i2c_dev_write(dev, 0xF4, 0x34 + (ctx->oversampling_setting << 6));
   if (status < 0)
   {
      goto out;
   }

   /* wait for the end of conversion: */
   msleep(2 + (3 << ctx->oversampling_setting << 1));

   /* read pressure values: */
   uint8_t data[3];
   for (int i = 0; i < 2; i++)
   {
      status = i2c_dev_read(dev, 0xF6 + i);
      if (status < 0)
      {
         goto out;
      }
      data[i] = (uint8_t)status;
   }

   /* swap positions to correct the MSB/LSB positions */
   ctx->raw_pressure = (data[0] << 16) | (data[1] << 8) | data[2];
   ctx->raw_pressure = ctx->raw_pressure >> (8 - ctx->oversampling_setting);

out:
   i2c_dev_unlock_bus(dev);

   return status;
}


static int bmp085_update_raw_temperature(i2c_dev_t *dev, bmp085_ctx_t *ctx)
{

   ASSERT_NOT_NULL(dev);
   ASSERT_NOT_NULL(ctx);

   i2c_dev_lock_bus(dev);

   /* start temperature conversion: */
   int status = i2c_dev_write(dev, 0xF4, 0x2E);
   if (status < 0)
   {
      goto out;
   }

   /* wait for the end of conversion: */
   msleep(5);

   /* read temperature values: */
   uint8_t data[2];
   for (int i = 0; i < 2; i++)
   {
      status = i2c_dev_read(dev, 0xF6 + i);
      if (status < 0)
      {
         goto out;
      }
      data[i] = (uint8_t)status;
   }

   /* data was read successfully, update context: */
   ctx->raw_temperature = (data[0] << 8) + data[1];

out:
   i2c_dev_unlock_bus(dev);

   return status;
}

