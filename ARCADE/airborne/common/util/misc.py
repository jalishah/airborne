
#
# misc.py
# misc functions and classes
# author: Tobias Simon, Ilmenau University of Technology
#


from sys import exit
from signal import pause
from daemon import DaemonContext
try:
   from daemon import pidlockfile
except:
   from daemon import pidfile as pidlockfile
from threading import Thread
from os import makedirs, getenv, sep
import errno
from time import time



def start_daemon_thread(target):
   thread = Thread(target = target)
   thread.daemon = True
   thread.start()
   return thread


def await_signal():
   try:
      pause()
   except:
      print 'killed by user'


def user_data_dir():
   '''
   $HOME/.ARCADE is used for storing device-specific files:
   - configuration files
   - logfiles
   - calibration data
   '''
   path = getenv('HOME') + sep + '.ARCADE'
   try:
      makedirs(path)
   except OSError, e:
      if e.errno != errno.EEXIST:
         raise
   return path


class Hysteresis:

   def __init__(self, timeout):
      self.timeout = timeout
      self.start_time = None

   def set(self):
      if self.start_time == None:
         self.start_time = time()
      elif self.start_time + self.timeout < time():
         return True
      return False
   
   def reset(self):
      self.start_time = None


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

