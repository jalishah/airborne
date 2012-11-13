
#include <math.h>

#include <util.h>

#include "force2twi.h"
#include "holger_blmc.h"
#include "../../../control/basic/control_param.h"


int force2twi_calc(uint8_t *i2c, const float voltage, const float *rpm_square, const size_t n_motors)
{
   int int_enable = 1;
   /* computation i2c values out of rpm_square by the inverse of: rpm ^ 2 = a * voltage ^ 1.5 * i2c ^ b */
   FOR_N(i, n_motors)
   {
      float temp = powf((rpm_square[i] / CTRL_F_A * powf(voltage, -1.5f)), 1.0f / CTRL_F_B);
      if (temp > (float)HOLGER_I2C_MIN)
      {   
         if (temp > (float)HOLGER_I2C_MAX)
         {   
            i2c[i] = (uint8_t)HOLGER_I2C_MAX;
            int_enable = 0;
         }
         else
         {   
            i2c[i] = (uint8_t)temp;
         }
      }
      else
      {   
         i2c[i] = (uint8_t)HOLGER_I2C_MIN;
         int_enable = 0;
      }
   }
   return int_enable;
}

