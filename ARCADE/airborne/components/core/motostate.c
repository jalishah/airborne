

#include "motostate.h"


/* motors state representation */
static enum 
{
   MOTORS_HALTED,
   MOTORS_STARTING,
   MOTORS_SPINNING,
   MOTORS_STOPPING
}
state;

static float timer;
static float gas_start;
static float gas_stop;
static float ground_max;


static int motors_start_condition(float ground_z, flight_state_t flight_state, float gas)
{
   return    ground_z <= ground_max 
          && gas >= gas_start 
          && flight_state == FS_STANDING;
}


static int motors_stop_condition(float ground_z, flight_state_t flight_state, float gas)
{
   return ground_z <= ground_max
          && gas <= gas_stop
          && flight_state != FS_FLYING;
}


static void reset_timer(void)
{
   timer = 0.0f;
}


static int timer_expired(float dt)
{
   const float TIMER_MAX = 4.0f;
   timer += dt;
   if (timer > TIMER_MAX)
   {
      timer = TIMER_MAX;
   }
   return timer == TIMER_MAX;
}


void motostate_init(float _ground_max, float _gas_start, float _gas_stop)
{
   ground_max = _ground_max;
   gas_start = _gas_start;
   gas_stop = _gas_stop;
   state = MOTORS_HALTED;
   timer = 0.0f;
}


int motostate_enabled(void)
{
   return (state == MOTORS_SPINNING) || (state == MOTORS_STARTING);
}


int motostate_controllable(void)
{
   return state == MOTORS_SPINNING;
}


void motostate_update(float ground_z, flight_state_t flight_state, float gas, float dt, int start_allowed)
{
   switch (state)
   {
      case MOTORS_HALTED:
         if (motors_start_condition(ground_z, flight_state, gas) && start_allowed)
         {
            state = MOTORS_STARTING;
            reset_timer();
         }
         break;
      
      case MOTORS_STARTING:
         if (motors_stop_condition(ground_z, flight_state, gas))
         {
            state = MOTORS_STOPPING;
            reset_timer();
         }
         else if (timer_expired(dt))
         {
            state = MOTORS_SPINNING;
         }
         break;
      
      case MOTORS_SPINNING:
         if (motors_stop_condition(ground_z, flight_state, gas))
         {
            state = MOTORS_STOPPING;
            reset_timer();
         }
         break;
      
      case MOTORS_STOPPING:
         if (motors_start_condition(ground_z, flight_state, gas))
         {
            state = MOTORS_STARTING;
            reset_timer();
         }
         else if (timer_expired(dt))
         {
            state = MOTORS_HALTED;
            reset_timer();
         }
         break;
   }
}

