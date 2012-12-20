

#include <daemon.h>

#include "main_loop/main_util.h"
#include "main_loop/main_realtime.h"
#include "main_loop/main_replay.h"


int main(int argc, char *argv[])
{
   int replay = 1;
   if (replay)
   {
      main_replay("/home/tobi/debug_data.msgpack");
   }
   else
   {
      main_realtime(argc, argv);
      daemonize("/var/run/core.pid", main_realtime, die, argc, argv);
   }
   return 0;
}

