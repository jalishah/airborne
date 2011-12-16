
/**
 * MADC userspace driver for Overo
 *
 * Tobias Simon, 2010
 * Parts from Hugo Vincent, May 2009
 */


#ifndef TWL4030_MACD_H
#define TWL4030_MACD_H


typedef struct twl4030_madc
{
   int fd;
   int open;
}
twl4030_madc_t;


typedef enum
{
   TWL4030_ADC_2,
   TWL4030_ADC_3,
   TWL4030_ADC_4,
   TWL4030_ADC_5,
   TWL4030_ADC_6,
   TWL4030_ADC_7,
   TWL4030_ADC_USB,
   TWL4030_ADC_RAIL
}
twl4030_madc_cannel_t;


void twl4030_madc_init(twl4030_madc_t *macd);


int twl4030_madc_open(twl4030_madc_t *macd);


int twl4030_madc_close(twl4030_madc_t *macd);


int twl4030_madc_convert(float *voltage_out, twl4030_madc_t *macd, twl4030_madc_cannel_t channel);


#endif /* TWL4030_MACD_H */

