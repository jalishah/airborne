
#
# misc.py
#
# Copyright (C) 2012 Tobias Simon, Ilmenau University of Technology
# parts taken from PsychoPy library, Copyright (C) 2009 Jonathan Peirce
#
# Distributed under the terms of the GNU General Public License (GPL).
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
import ctypes, ctypes.util



# user data directory:

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
   except OSError as e:
      if e.errno != errno.EEXIST:
         raise
   return path


# Hysteresis class:

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



# process and thread daemonization:

def _main_wrapper(name, main):
   main(name)
   pause()


def daemonize(name, main):
   try:
      pidf = pidlockfile.PIDLockFile('/var/run/' + name + '.pid')
      pidf.acquire(timeout = 1.0)
      pidf.release()
      with DaemonContext(pidfile = pidf):
         _main_wrapper(name, main)
   except Exception as e:
      print 'Could not daemonize:', str(e)
      exit(1)


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




# process priority modification:

_c = ctypes.cdll.LoadLibrary(ctypes.util.find_library('c'))

_SCHED_FIFO = 1

class _SchedParams(ctypes.Structure):
   _fields_ = [('sched_priority', ctypes.c_int)]


def sched_get_minprio():
   return _c.sched_get_priority_min(_SCHED_FIFO)


def sched_get_maxprio():
   return _c.sched_get_priority_max(_SCHED_FIFO)


def sched_rtprio(priority):
    priority = int(priority)
    if priority > sched_get_maxprio():
       raise ValueError('priority too high')
    elif priority < sched_get_minprio():
       raise ValueError('priority too high')
    schedParams = _SchedParams()
    schedParams.sched_priority = priority
    err = _c.sched_setscheduler(0, _SCHED_FIFO, ctypes.byref(schedParams))
    if err != 0:
      raise OSError('could not set priority, code: %d' % err)

