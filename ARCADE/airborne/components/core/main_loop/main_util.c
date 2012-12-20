
#include <signal.h>
#include <unistd.h>

#include "main_util.h"
#include "../util/logger/logger.h"



void die(void)
{
   static int killing = 0;
   if (!killing)
   {
      killing = 1;
      LOG(LL_INFO, "shutting down");
      sleep(1);
      kill(0, 9);
   }
}


