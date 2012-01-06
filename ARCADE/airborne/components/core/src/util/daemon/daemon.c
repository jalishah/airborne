
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>


#include "pidfile.h"


char *pid_path = "/var/run/core.pid";
extern void _main(int argc, char *argv[]);
extern void _cleanup(void);


void term_handler(int sig)
{
   (void)sig;
   remove_pid(pid_path);
   _cleanup();
   sleep(1);
   exit(EXIT_SUCCESS);
}


void daemon_main(int argc, char *argv[])
{
   struct sigaction new, old;
   new.sa_handler = term_handler;
   sigemptyset(&new.sa_mask);
   new.sa_flags = 0;
   sigaction(SIGINT, &new, &old);
   sigaction(SIGTERM, &new, &old);
   write_pid(pid_path);
   _main(argc, argv);
}


int main(int argc, char *argv[])
{
#if 1
   if (check_pid(pid_path))
   {
      exit(EXIT_FAILURE);
   }

   /* Our process ID and Session ID */
   pid_t pid, sid;

   /* Fork off the parent process */
   pid = fork();
   if (pid < 0)
   {
      exit(EXIT_FAILURE);
   }
   /* If we got a good PID, then
      we can exit the parent process. */
   if (pid > 0)
   {
      exit(EXIT_SUCCESS);
   }

   /* Change the file mode mask */
   umask(0);

   /* Create a new SID for the child process */
   sid = setsid();
   if (sid < 0)
   {
      exit(EXIT_FAILURE);
   }

   /* Change the current working directory */
   if ((chdir("/")) < 0)
   {
      exit(EXIT_FAILURE);
   }

   /* Close and re-open the standard file descriptors */
   close(STDIN_FILENO);
   close(STDOUT_FILENO);
   close(STDERR_FILENO);

   open("/dev/null", O_RDONLY); /* stdin */
   open("/dev/null", O_WRONLY); /* stderr */
   open("/dev/null", O_RDWR); /* stdout */
#endif
   /* Daemon-specific initialization goes here */
   daemon_main(argc, argv);

   exit(EXIT_SUCCESS);
}

