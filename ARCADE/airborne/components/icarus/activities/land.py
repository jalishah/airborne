
from time import sleep
from core_pb2 import *

from activity import Activity


class LandActivity(Activity):

   MIN_HOVERING_ALT = 0.57

   def __init__(self, fsm, core, mon):
      Activity.__init__(self)
      self.fsm = fsm
      self.core = core
      self.mon = mon

   def run(self):
      self.core.set_ctrl_param(POS_Z_GROUND, self.MIN_HOVERING_ALT / 3.0)
      while self.mon.z_ground > self.MIN_HOVERING_ALT:
         sleep(self.POLLING_TIMEOUT)
      self.core.spin_down()
      self.fsm.landing_done()

