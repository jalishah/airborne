

#include "timer.h"


void timer_init(timer_t *timer, float expire)
{
   timer->expire = expire;
   timer_reset(timer);
}


void timer_reset(timer_t *timer)
{
   timer->state = 0.0f;
}


int timer_check(timer_t *timer, float dt)
{
   timer->state += dt;
   if (timer->state > timer->expire)
   {
      timer->state = timer->expire;
   }
   return timer->state == timer->expire;
}

