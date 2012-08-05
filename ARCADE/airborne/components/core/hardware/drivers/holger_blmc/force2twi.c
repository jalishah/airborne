
#include "force2twi.h"
#include <math.h>



int round(float x)
{
   int result;
   if (x > 0)
   {
      result = (int)(x + 0.5);
   }
   else
   {
      result = (int)(x - 0.5);
   }
   return result;
}



typedef struct
{
   float *rpm_square;
   coupling *coupling;
}
force2twi_ctx_t;


unsigned int force2twi(force2twi_ctx_t ctx, float *force, float voltage, unsigned char *i2c, unsigned int motors)
{
   unsigned int int_enable = 0;

   /* computation of rpm ^ 2 out of the desired forces */
   coupling_calc(cts->coupling,  ctx->rpm_square, force);
            
   /* computation i2c values out of rpm_square by the inverse of: rpm ^ 2 = a * voltage ^ 1.5 * i2c ^ b */
   for (unsigned int i = 0; i < 4; i++)
   {     
      int temp = round(powf((rpm_square[i] / CTRL_F_A * powf(voltage, -1.5f)), 1.0f / CTRL_F_B));
      if (temp < I2C_MIN)
      {
         temp = I2C_MIN;
      }
      else if (temp > I2C_MAX)
      {
         temp = I2C_MAX;
      }
      else
      {
         int_enable = 1;
      }
      i2c[i] = temp;
   }

   return int_enable;
}

