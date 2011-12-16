
#include "maxsonar.h"
#include "../../util/logger/logger.h"

#include <stdint.h>
#include <ctype.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <assert.h>


struct maxsonar
{
   enum
   {
      READ_R,
      READ_DATA,
      READ_CR
   } state;

   uint8_t data[3];
   int data_i;

   float dist;
};


maxsonar_t *maxsonar_create(void)
{
   maxsonar_t *sonar = malloc(sizeof(maxsonar_t));
   sonar->dist = 0.2; /* minimum value for sensor */
   sonar->data_i = 0;
   sonar->state = READ_R;
   return sonar;
}



int maxsonar_parse(maxsonar_t *sonar, uint8_t b)
{
   int status = 0;
   switch (sonar->state)
   {
      case READ_R:
         if (b != 'R')
         {
            LOG(LL_DEBUG, "expected R, got: %c", b);
            status = -1;
         }
         else
         {
            sonar->state = READ_DATA;
            status = 0;
         }
         sonar->data_i = 0;
         break;

      case READ_DATA:
         if (!isdigit(b))
         {
            LOG(LL_DEBUG, "expected digit, got: %c", b);
            sonar->state = READ_R;
            status = -1;
         }
         else
         {
            sonar->data[sonar->data_i] = b;
            sonar->data_i++;
            if (sonar->data_i == 3)
            {
               sonar->state = READ_CR;
            }
         }
         break;

      case READ_CR:
         if (b != '\r')
         {
            LOG(LL_DEBUG, "expected CR, got: %c", b);
            status = -1;
         }
         else
         {
            if (sonar->data_i != 3)
            {
               LOG(LL_DEBUG, "at CR: invalid index: %d", sonar->data_i);
               status = -1;
            }
            else
            {
               uint16_t dist = 0;
               uint16_t dec_pos = 100;
               for (int i = 0; i < 3; i++)
               {
                  uint16_t c = sonar->data[i] - '0';
                  dist += c * dec_pos;
                  dec_pos /= 10;
               }
               if (dist < 20 || dist > 800)
               {
                  LOG(LL_DEBUG, "distance does not make sense: %d", dec_pos);
                  status = -1;
               }
               else
               {
                  sonar->dist = (float)dist / 100.0f;
                  status = 1;
               }
            }
         }
         sonar->state = READ_R;
         break;
   }
   return status;
}


float maxsonar_get_dist(maxsonar_t *sonar)
{
   return sonar->dist;
}

