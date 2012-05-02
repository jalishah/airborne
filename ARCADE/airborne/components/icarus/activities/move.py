
from time import sleep
from threading import Thread, Timer, current_thread
from core_pb2 import *
from math import sqrt

from activities import Activity, StabMixIn


class MoveActivity(Activity, StabMixIn):


   Z_MAX = 2.0


   def __init__(self, fsm, core, msg):
      Activity.__init__(self)
      StabMixIn.__init__(self, core)
      self.fsm = fsm
      self.core = core
      self.msg = msg
      self.canceled = False


   def run(self):
      # get position setpoints from message:
      x_or_y_set = False
      if self.arg.move_data.p0 != None:
         x_or_y_set = True
         self.core.set_ctrl_param(POS_X, self.arg.move_data.p0)
      if self.arg.move_data.p1 != None:
         x_or_y_set = True
         self.core.set_ctrl_param(POS_Y, self.arg.move_data.p1)
      
      if x_or_y_set:
         # set speed:
         self.core.set_ctrl_param(SPEED_XY, self.arg.speed)
         self.core.set_ctrl_param(SPEED_Z, self.Z_MAX)
         # update z setpoint linearly between the the starting position and the destination:
         n = lineq_n(0.0, self.mon.z, self.arg.move_data.p1)
         m = self.mon_z
         while not self.arrived:
            if self.cancel:
               self.core.set_ctrl_param(POS_X, self.mon.x)
               self.core.set_ctrl_param(POS_Y, self.mon.y)
               self.stabilize()
               return # not going into hovering state
            dist = euclid_dist(self.arg.move_data.p0)
            alt = n * x + m
            self.core.set_ctrl_param(POS_Z, alt)
            sleep(1)
      else:
         # only z position is updated: set speed and position directly:
         self.core.set_ctrl_param(SPEED_Z, self.arg.speed)
         if self.arg.move_data.p2 != None:
            self.core.set_ctrl_param(POS_Z, self.arg.move_data.p2)
 
      self.stabilize()
      self.fsm.done()

   def cancel(self):
      self.canceled = True

