
/**
 * MADC userspace driver for Overo
 *
 * Tobias Simon, 2010
 * Parts from Hugo Vincent, May 2009
 */

#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>

typedef uint8_t u8;
typedef uint16_t u16;

#include "util.h"
#include "i2c_twl4030-madc.h"
#include "twl4030-madc.h"


typedef struct
{
   int number;
   char *name;
   float input_range;
} adc_channel_t;



/**
 * channel numbering:
 * ADC0-1 : to do with battery charging, not relevant on Overo
 * ADC2-7 : general purpose, input range = 0 - 2.5V.
 *          On Overo, ADC2 seems to read as ~0.4 and ADC7 as ~1.4V (?).
 * ADC8 : USB OTG port bus voltage.
 * ADC9-11 : more battery charging stuff, not relevant.
 * ADC12 : main battery voltage.
 *         This will be the system 3.3V rail in our case
 * ADC13-15: reserved or not relevant.
 */

static adc_channel_t channels[] =
{
   {
      .number = 2,
      .name = "ADCIN2",
      .input_range = 2.5,
   },
   {
      .number = 3,
      .name = "ADCIN3",
      .input_range = 2.5,
   },
   {
      .number = 4,
      .name = "ADCIN4",
      .input_range = 2.5,
   },
   {
      .number = 5,
      .name = "ADCIN5",
      .input_range = 2.5,
   },
   {
      .number = 6,
      .name = "ADCIN6",
      .input_range = 2.5,
   },
   {
      .number = 7,
      .name = "ADCIN7",
      .input_range = 2.5,
   },
   {
      .number = 8,
      .name = "VBUS_USB_OTG",
      .input_range = 7.0,
   },
   {
      .number = 12,
      .name = "VBATT/3.3V_RAIL",
      .input_range = 6.0,
   },
};


void twl4030_madc_init(twl4030_madc_t *macd)
{
   macd->open = 0;
}


int twl4030_madc_open(twl4030_madc_t *macd)
{
   ASSERT_NOT_NULL(macd);
   ASSERT_FALSE(macd->open);

   int fd = open("/dev/twl4030-madc", O_RDWR | O_NONBLOCK);
   if (fd == -1)
   {
      return -1;
   }
   macd->fd = fd;
   macd->open = 1;
   return 0;
}


int twl4030_madc_close(twl4030_madc_t *macd)
{
   ASSERT_NOT_NULL(macd);
   ASSERT_TRUE(macd->open);

   int err = close(macd->fd);
   if (err == 0)
   {
      macd->open = 0;
   }
   return err;
}


int twl4030_madc_convert(float *voltage_out, twl4030_madc_t *macd, twl4030_madc_cannel_t channel)
{
   ASSERT_NOT_NULL(macd);
   ASSERT_TRUE(macd->open);

   struct twl4030_madc_user_parms par;
   par.channel = channels[channel].number;
   int ret = ioctl(macd->fd, TWL4030_MADC_IOCX_ADC_RAW_READ, &par);
   float voltage = ((unsigned int)par.result) / 1024.0f * channels[channel].input_range;
   if (!(ret == 0 && par.status != -1))
   {
      return -1;
   }
   *voltage_out = voltage;
   return 0;
}

