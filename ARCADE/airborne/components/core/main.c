
#include <stdlib.h>
#include <daemon.h>

#include "main_loop/main_util.h"
#include "main_loop/main_realtime.h"
#include "main_loop/main_replay.h"


int main(int argc, char *argv[])
{
   char *file = NULL;
   if (argc > 1)
   {
      file = argv[1];
   }

   if (file)
   {
      printf("replaying %s\n", file);
      main_replay(file);
   }
   else
   {
      main_realtime(argc, argv);
      daemonize("/var/run/core.pid", main_realtime, die, argc, argv);
   }
   return 0;
}

