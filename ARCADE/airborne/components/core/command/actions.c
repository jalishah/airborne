
/*
 * purpose: actions implementation
 * author: Tobias Simon, Ilmenau University of Technology
 */


#include "actions.h"
#include "util.h"
#include "../util/logger/logger.h"
#include "../control/ctrl.h"
#include "../platform/platform.h"


static float *rpm = NULL;


int actions_init(void)
{
   ASSERT_ONCE();
   rpm = malloc(platform->motors() * sizeof(float));
   return 0;
}


int action_spin_up(void)
{
   int retry_count = 0;

retry:
   LOG(LL_DEBUG, "starting motors");
   ctrl_override(0.0f, 0.0f, 0.0f, 0.0f);
   fc_start_motors();
   sleep(1);
   for (int i = 0; i < 6; i++)
   {
      ctrl_override(0.0f, 0.0f, 0.0f, (i & 1)? 0.3f : 0.1f);
   }
   int valid_count = 0;
   int fail_timer = 1000;
   
   while (1)
   {
      msleep(10);
      valid_count++;
      
      if (fail_timer-- == 0)
      {
          LOG(LL_ERROR, "starting failed, restarting");
          spin_down();
          if (++retry_count == 3)
          {
             return -1;
          }
          goto retry;
      }
      
      /* reset valid_count, if motor rpm too low */
      fc_read_motors_rpm(rpm);
      for (int i = 0; i < platform_motors(); i++)
      {
         if (rpm[i] < 1000.0f || rpm[i] > 50000.0f)
         {
            valid_count = 0;
            break;
         }
      }

      if (valid_count > 200)
      {
         LOG(LL_ERROR, "motors are running");
         ctrl_stop_override();
         return 0;
      }
   }
}


void action_spin_down(void)
{
   /* switch off motors command: */
   fc_stop_motors();
   
   /* wait until all motors are stopped: */
   int done = 0;
   while (!done)
   {
      msleep(100);
      fc_read_motors_rpm(rpm);
      for (int i = 0; i < platform_motors(); i++)
      {
         if (rpm[i] > 10.0)
         {
            done = 1;
            break;
         }
      }
   }
   LOG(LL_ERROR, "motors are stopped");
}

