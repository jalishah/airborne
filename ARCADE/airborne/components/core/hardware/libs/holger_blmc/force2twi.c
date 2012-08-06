

#include <malloc.h>
#include <math.h>

#include "force2twi.h"
#include "holger_blmc.h"
#include "../../../control/basic/control_param.h"


static float *rpm_square;
static const coupling_t *coupling;


void force2twi_init(const coupling_t *coup)
{
   rpm_square = malloc(coupling_motors(coup) * sizeof(float));
   coupling = coup;
}


bool force2twi_calc(const float *force, const float voltage, uint8_t *i2c)
{
   bool int_enable = false;

   /* computation of rpm ^ 2 out of the desired forces */
   coupling_calc(coupling, rpm_square, force);
            
   /* computation i2c values out of rpm_square by the inverse of: rpm ^ 2 = a * voltage ^ 1.5 * i2c ^ b */
   for (unsigned int i = 0; i < coupling_motors(coupling); i++)
   {     
      int temp = round(powf((rpm_square[i] / CTRL_F_A * powf(voltage, -1.5f)), 1.0f / CTRL_F_B));
      if (temp < HOLGER_I2C_MIN)
      {
         temp = HOLGER_I2C_MIN;
      }
      else if (temp > HOLGER_I2C_MAX)
      {
         temp = HOLGER_I2C_MAX;
      }
      else
      {
         int_enable = true;
      }
      i2c[i] = temp;
   }
   return int_enable;
}

