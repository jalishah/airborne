
#include <stdio.h>
#include <math.h>
#include <unistd.h>

#include "../sensor_actor/lib/twl4030-madc/twl4030-madc.h"


int main(void)
{
   twl4030_madc_t madc;
   twl4030_madc_init(&madc);
   twl4030_madc_open(&madc);
   while (1)
   {
      float voltage;
      twl4030_madc_convert(&voltage, &madc, TWL4030_ADC_3);
      float distance = 65.0 * pow(voltage, -1.10);
      printf("%f\n", distance);
      fflush(stdout);
      usleep(10000);
   }
   return 0;
}
