
from core_pb2 import *
from activity import Activity, StabMixIn


class StopActivity(Activity, StabMixIn):

   def __init__(self, fsm, core):
      Activity.__init__(self)
      StabMixIn.__init__(self, core)
      self.fsm = fsm
      self.core = core

   def run(self):
      self.core.reset_setpoint(GPS_METERS)
      self.stabilize()

