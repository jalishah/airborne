
#ifndef __TIMER_H__
#define __TIMER_H__


/* timer structure */
typedef struct
{
   float expire;
   float state;
}
timer_t;

/* initializes a timer to expire in a given amout of time in seconds */
void timer_init(timer_t *timer, float expire);


/* resets the timer  */
void timer_reset(timer_t *timer);


/* advances the timer by dt seconds
   returns 1 if the timer expired and 0 if not */
int timer_check(timer_t *timer, float dt);


#endif /* __TIMER_H__ */

