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
      self._core = core

   def stabilize(self):
      core = self._core
      count = 0
      while True:
         count += 1
         if count == self.STAB_COUNT:
            break
         if self._canceled:
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
      self._sm = sm
      self._core = core
      self._arg = arg
      self._canceled = False

   def _cancel(self):
      self._canceled = True

   def run(self):
      sm = self._sm
      core = self._core
      core.set_ctrl_param(POS_Z_MSL, self.LOW_ALT_SETPOINT, RELATIVE)
      core.power_on()

      if self._canceled:
         return

      # "point of no return":
      try:
         core.start_motors()
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
      sm.done()


class LandActivity(Activity):

   MIN_HOVERING_ALT = 0.57


   def __init__(self, fsm, core):
      Activity.__init__(self)
      self._fsm = fsm
      self._core = core

   def run(self):
      self._core.set_altitude(ALT, self.MIN_HOVERING_ALT / 3.0, RELATIVE)
      while self._core.get_state(ULTRA_ALT) > self.MIN_HOVERING_ALT:
         sleep(POLLING_TIMEOUT)
      self._core.stop_motors()
      self._fsm.landing_done()


class MoveActivity(Activity, StabMixIn):

   def __init__(self, fsm, core, arg):
      Activity.__init__(self)
      StabMixIn.__init__(self, core)
      self._fsm = fsm
      self._core = core
      self._arg = arg
      self._canceled = False

   def run(self):
      self._core.set_gps((self._arg[0], self._arg[1]))
      self.stabilize()
      self._fsm.done()

   def _cancel(self):
      self._canceled = True


class StopActivity(Activity, StabMixIn):

   def __init__(self, fsm, core):
      Activity.__init__(self)
      StabMixIn.__init__(self, core)
      self._fsm = fsm
      self._core = core

   def run(self):
      self._core.reset_setpoint(GPS_METERS)
      self.stabilize()

