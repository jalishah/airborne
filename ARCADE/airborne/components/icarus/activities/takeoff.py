
from core_pb2 import *
from activity import Activity, StabMixIn

from logging import debug as log_debug, info as log_info, warning as log_warn, error as log_err



class TakeoffActivity(Activity, StabMixIn):

   LOW_ALT_SETPOINT = -10.0
   STD_HOVERING_ALT = 3.0


   def __init__(self, fsm, icarus):
      Activity.__init__(self, icarus)
      self.canceled = False
      self.fsm = fsm


   def _cancel(self):
      self.canceled = True


   def run(self):
      arg = self.icarus.arg
      core = self.icarus.core
      mon_data = self.icarus.mon_data
      params = self.icarus.core.params

      if arg.HasField('move_data'):
         z_setpoint = arg.move_data.z
         if arg.HasField('rel'):
            log_warn('rel field ignored for take-off')
         if arg.HasField('glob'):
            if not arg.glob:
               if z_setpoint < core.params.start_alt + mon_data.z + 3.0:
                  msg = 'absolute z setpoint %f is below current altitude' % z_setpoint
                  log_err(msg)
                  raise ValueError(msg)
               log_info('taking off to absolute altitude %f' % z_setpoint)
            else:
               z_setpoint = mon_data.z + z_setpoint
               log_info('taking off to relative altitude %f' % z_setpoint)
      else:
         z_setpoint = self.STD_HOVERING_ALT

      try:
         core.spin_up()
      except:
         core.spin_down()
         self.fsm.failed()
         log_error('could not spin up motors');
         return

      if self.canceled:
         core.spin_down()
         log_error('take-off canceled');
         return

      # "point of no return":
      # reset controllers:
      core.set_ctrl_param(POS_YAW, mon_data.yaw)
      core.set_ctrl_param(POS_X, mon_data.x)
      core.set_ctrl_param(POS_Y, mon_data.y)
      core.reset_ctrl()

      # set new altitude setpoint and stabilize:
      core.set_ctrl_param(POS_Z, z_setpoint)
      self.stabilize()
      self.fsm.done()

