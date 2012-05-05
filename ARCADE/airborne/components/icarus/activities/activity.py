
from time import sleep
from threading import Thread, current_thread
from core_pb2 import *
from math import sqrt


class Activity(Thread):

   def __init__(self):
      Thread.__init__(self)
      self.daemon = True

   def _cancel(self):
      pass

   def cancel_and_join(self):
      if self != current_thread():
         self._cancel()
         self.join()


class StabMixIn:
   
   LAT_STAB_EPSILON = 3.0
   ALT_STAB_EPSILON = 0.4
   YAW_STAB_EPSILON = 0.3
   POLLING_TIMEOUT = 0.1
   STAB_COUNT = 20

   def __init__(self, core):
      self.core = core

   def stabilize(self):
      core = self.core
      count = 0
      while True:
         count += 1
         if count == self.STAB_COUNT:
            break
         if self.canceled:
            return
         sleep(self.POLLING_TIMEOUT)
         # read error values from core:
         x_err, y_err = core.mon.x_err, core.mon.y_err
         alt_err = core.mon.z_err
         yaw_err = core.mon.yaw_err
         # reset counter if one of the errors becomes too huge:
         if alt_err > self.ALT_STAB_EPSILON:
            print 'alt instable', alt_err, count
            count = 0
         elif sqrt(x_err * x_err + y_err * y_err) > self.LAT_STAB_EPSILON:
            print 'gps instable', x_err, y_err, count
            count = 0
         elif yaw_err > self.YAW_STAB_EPSILON:
            print 'yaw instable', core.get_ctrl_error(YAW), count
            count = 0
      print 'stabilized'

