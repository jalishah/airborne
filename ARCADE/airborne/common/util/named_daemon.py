# named daemon

from sys import exit
from signal import pause
from daemon import DaemonContext
from daemon import pidfile


def main_wrapper(name, main):
   main(name)
   pause()


def daemonize(name, main):
   try:
      pidf = pidfile.PIDLockFile('/var/run/' + name + '.pid')
      pidf.acquire(timeout = 1.0)
      pidf.release()
      with DaemonContext(pidfile = pidf):
         main_wrapper(name, main)
   except:
      exit(1)

