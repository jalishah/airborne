from time import sleep
from threading import Thread, Timer, current_thread
from core_pb2 import *
from math import sqrt


# altitude control setpoints:
LOW_ALT_SETPOINT = -10.0
MIN_HOVERING_ALT = 0.57
STD_HOVERING_ALT = 0.5

# stabilization counter and waiting time:
POLLING_TIMEOUT = 0.1
STAB_COUNT = 20

# epsilon values for stabilizing controllers:
LAT_STAB_EPSILON = 3.0
ALT_STAB_EPSILON = 0.4
YAW_STAB_EPSILON = 0.3

POWERSAVE_TIMEOUT = 60 * 5


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

   def __init__(self, core):
      self._core = core

   def stabilize(self):
      count = 0
      while True:
         count += 1
         if count == STAB_COUNT:
            break
         if self._canceled:
            return
         sleep(POLLING_TIMEOUT)
         core = self._core
         x, y = core.get_ctrl_error(GPS)
         if core.get_ctrl_error(ALT) > ALT_STAB_EPSILON:
            print 'alt instable', core.get_ctrl_error(ALT)
            count = 0
         if sqrt(x ** 2.0 + y ** 2.0) > LAT_STAB_EPSILON:
            print 'gps instable', x, y
            count = 0
         if core.get_ctrl_error(YAW) > YAW_STAB_EPSILON:
            print 'yaw instable', core.get_ctrl_error(YAW)
            count = 0
      print 'stabilized'


class DummyActivity(Activity):

   def __init__(self):
      Activity.__init__(self)

   def run(self):
      pass


class PowerSaveActivity(Activity):

   def __init__(self, ctrl):
      Activity.__init__(self)
      self.ctrl = ctrl

   def _cancel(self):
      self.timer.cancel()
      self.timer.join()

   def run(self):
      self.timer = Timer(POWERSAVE_TIMEOUT, self.ctrl.power_off)
      self.timer.start()
      self.timer.join()


class TakeoffActivity(Activity, StabMixIn):

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
      core.set_altitude(LOW_ALT_SETPOINT, RELATIVE)
      core.power_on()

      if self._canceled:
         return

      # "point of no return", cancellation not possible anymore
      try:
         core.start_motors()
      except:
         sm.failed()
         return

      core.set_yaw(core.get_state(YAW_POS))
      core.set_gps(core.get_state(GPS_REL))
      core.reset_controllers()

      if self._arg.alt:
         core.set_altitude(self._arg.alt, RELATIVE)
      else:
         core.set_altitude(STD_HOVERING_ALT, RELATIVE)

      self.stabilize()
      sm.done()


class LandActivity(Activity):

   def __init__(self, fsm, core):
      Activity.__init__(self)
      self._fsm = fsm
      self._core = core

   def run(self):
      self._core.set_altitude(ALT, MIN_HOVERING_ALT / 3.0, RELATIVE)
      while self._core.get_state(ULTRA_ALT) > MIN_HOVERING_ALT:
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

