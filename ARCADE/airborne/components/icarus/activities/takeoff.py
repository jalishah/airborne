
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
      arg = self.arg
      core = self.core
      mon = self.mon
      params = core.params

      if arg.HasField('move_data'):
         z_setpoint = arg.move_data.z
         if arg.HasField('rel'):
            log_warn('rel field ignored for take-off')
         if arg.HasField('glob'):
            if not arg.glob:
               if z_setpoint < core.params.start_alt + mon.z + 3.0:
                  msg = 'absolute z setpoint %f is below current altitude' % z_setpoint
                  log_err(msg)
                  raise ValueError(msg)
               log_info('taking off to absolute altitude %f' % z_setpoint)
            else:
               z_setpoint = mon.z + z_setpoint
               log_info('taking off to relative altitude %f' % z_setpoint)
      else:
         z_setpoint = self.STD_HOVERING_ALT

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
      # reset controllers:
      core.set_ctrl_param(POS_YAW, mon.yaw)
      core.set_ctrl_param(POS_X, mon.x)
      core.set_ctrl_param(POS_Y, mon.y)
      core.reset_controllers()

      # set new altitude setpoint and stabilize:
      core.set_ctrl_param(POS_Z, z_setpoint)
      self.stabilize()
      self.fsm.done()

