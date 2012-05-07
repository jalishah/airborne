
from time import sleep
from threading import Thread, current_thread
from core_pb2 import *
from math import sqrt, hypot


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
   ALT_STAB_EPSILON = 10.4
   YAW_STAB_EPSILON = 0.3
   POLLING_TIMEOUT = 0.1
   STAB_COUNT = 20

   def __init__(self, core):
      self.core = core

   def stabilize(self):
      core = self.core
      count = 0
      while True:
         sleep(self.POLLING_TIMEOUT)
         count += 1
         if count == self.STAB_COUNT:
            break
         if self.canceled:
            return
         # read error values from core:
         x_err, y_err = self.mon_data.x_err, self.mon_data.y_err
         alt_err = self.mon_data.z_err
         yaw_err = self.mon_data.yaw_err
         # reset counter if one of the errors becomes too huge:
         print x_err, y_err
         if abs(alt_err) > self.ALT_STAB_EPSILON:
            print 'alt instable', alt_err, count
            count = 0
         elif hypot(x_err, y_err) > self.LAT_STAB_EPSILON:
            print 'gps instable', x_err, y_err, count
            count = 0
         elif abs(yaw_err) > self.YAW_STAB_EPSILON:
            print 'yaw instable', yaw_err, count
            count = 0
      print 'stabilized'

