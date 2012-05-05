
from core_pb2 import *
from activity import Activity, StabMixIn


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
      core.set_ctrl_param(POS_Z, self.LOW_ALT_SETPOINT)
      core.power_on()

      if self.canceled:
         return

      # "point of no return":
      try:
         core.spin_up()
      except:
         self.fsm.failed()
         return

      core.set_yaw(core.get_state(YAW_POS))
      core.set_gps(core.get_state(GPS_REL))
      core.reset_controllers()

      #if len(self._arg.alt:
      #   core.set_altitude(self._arg.alt, RELATIVE)
      #else:
      #   core.set_altitude(STD_HOVERING_ALT, RELATIVE)

      self.stabilize()
      self.fsm.done()

