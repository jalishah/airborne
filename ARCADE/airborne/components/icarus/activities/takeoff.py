
from core_pb2 import *
from activity import Activity, StabMixIn

from logging import log_config, debug as log_debug
from logging import info as log_info, warning as log_warn, error as log_err



class TakeoffActivity(Activity, StabMixIn):

   LOW_ALT_SETPOINT = -10.0
   STD_HOVERING_ALT = 0.5


   def __init__(self, fsm, core, arg):
      Activity.__init__(self)
      StabMixIn.__init__(self, core)
      self.fsm = fsm
      self.core = core
      self.arg = arg
      self.canceled = False


   def _cancel(self):
      self.canceled = True


   def run(self):
      core = self.core
      mon = self.mon
      core.set_ctrl_param(POS_Z, self.LOW_ALT_SETPOINT)

      try:
         core.spin_up()
      except:
         core.spin_down()
         self.fsm.failed()
         return

      if self.canceled:
         core.spin_down()
         return
      # "point of no return":
      
      core.set_ctrl_param(POS_YAW, mon.yaw)
      core.set_ctrl_param(POS_X, mon.x)
      core.set_ctrl_param(POS_Y, mon.y)
      core.reset_controllers()

      if len(self.arg.alt:
         core.set_altitude(self._arg.alt, RELATIVE)
      else:
         core.set_altitude(STD_HOVERING_ALT, RELATIVE)

      self.stabilize()
      self.fsm.done()

