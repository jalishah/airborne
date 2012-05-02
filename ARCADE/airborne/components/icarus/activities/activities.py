from time import sleep
from threading import Thread, Timer, current_thread
from core_pb2 import *
from math import sqrt


class Activity(Thread):

   def __init__(self):
      Thread.__init__(self)

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
         x_err, y_err = core.get_ctrl_error(GPS)
         alt_err = core.get_ctrl_error(ALT)
         yaw_err = core.get_ctrl_error(YAW)
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


class DummyActivity(Activity):

   def __init__(self):
      Activity.__init__(self)

   def run(self):
      pass


class PowerSaveActivity(Activity):

   POWERSAVE_TIMEOUT = 60 * 5

   def __init__(self, ctrl):
      Activity.__init__(self)
      self.ctrl = ctrl

   def _cancel(self):
      self.timer.cancel()
      self.timer.join()

   def run(self):
      self.timer = Timer(self.POWERSAVE_TIMEOUT, self.ctrl.power_off)
      self.timer.start()
      self.timer.join()


class TakeoffActivity(Activity, StabMixIn):

   LOW_ALT_SETPOINT = -10.0
   STD_HOVERING_ALT = 0.5


   def __init__(self, sm, core, arg):
      Activity.__init__(self)
      StabMixIn.__init__(self, core)
      self.fsm = sm
      self.core = core
      self.arg = arg
      self.canceled = False

   def _cancel(self):
      self.canceled = True

   def run(self):
      sm = self._sm
      core = self._core
      core.set_ctrl_param(POS_Z_MSL, self.LOW_ALT_SETPOINT)
      core.power_on()

      if self.canceled:
         return

      # "point of no return":
      try:
         core.spin_up()
      except:
         sm.failed()
         return

      core.set_yaw(core.get_state(YAW_POS))
      core.set_gps(core.get_state(GPS_REL))
      core.reset_controllers()

      #if len(self._arg.alt:
      #   core.set_altitude(self._arg.alt, RELATIVE)
      #else:
      #   core.set_altitude(STD_HOVERING_ALT, RELATIVE)

      self.stabilize()
      fsm.done()


class LandActivity(Activity):

   MIN_HOVERING_ALT = 0.57


   def __init__(self, fsm, core, mon):
      Activity.__init__(self)
      self.fsm = fsm
      self.core = core
      self.mon = mon

   def run(self):
      print 'x'
      self.core.set_ctrl_param(POS_Z_GROUND, self.MIN_HOVERING_ALT / 3.0)
      print 'x'
      #while self.mon.z_ground > self.MIN_HOVERING_ALT:
         #sleep(self.POLLING_TIMEOUT)
      self.core.spin_down()
      self.fsm.landing_done()


class StopActivity(Activity, StabMixIn):

   def __init__(self, fsm, core):
      Activity.__init__(self)
      StabMixIn.__init__(self, core)
      self.fsm = fsm
      self.core = core

   def run(self):
      self.core.reset_setpoint(GPS_METERS)
      self.stabilize()

