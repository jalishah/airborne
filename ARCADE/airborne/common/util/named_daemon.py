# named daemon

from sys import exit
from signal import pause
from daemon import DaemonContext

try:
   from daemon import pidlockfile
except:
   from daemon import pidfile as pidlockfile


def main_wrapper(name, main):
   main(name)
   pause()

def daemonize(name, main):
   try:
      pidf = pidlockfile.PIDLockFile('/var/run/' + name + '.pid')
      pidf.acquire(timeout = 1.0)
      pidf.release()
      with DaemonContext(pidfile = pidf):
         main_wrapper(name, main)
   except Exception, e:
      print 'Could not daemonize:', str(e)
      exit(1)

