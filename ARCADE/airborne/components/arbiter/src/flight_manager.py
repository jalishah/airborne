
#
# the flight manager is the central component of the arbiter
# it
# - takes care that the UAV system is always in a valid state by using the flight state machine
# - accepts or rejects user commands
# - publishes state updates via SCL
# - manages acoustic state and error signaling
#


from logging import Logger
from arbiter_pb2 import TAKEOFF, LAND, MOVE, STOP, ROT
from state_update_pb2 import StateUpdate
from activities.activities import DummyActivity, TakeoffActivity, LandActivity, MoveActivity, StopActivity
from flight_sm.flight_sm import flight_sm


class StateMachineError(Exception):
   pass


class FlightManager:

   """takes arbiter commands and translates them into core commands"""

   def __init__(self, core, sui):
      self._sui = sui
      self._core = core
      self._fsm = flight_sm(self)
      self.activity = DummyActivity()
      self.activity.start()


   # called externally from arbiter protocol driver
   def handle(self, req):
      self._arg = req
      if req.type == TAKEOFF:
         self._fsm.takeoff()
      elif req.type == LAND:
         self._fsm.land()
      elif req.type == MOVE:
         self._fsm.move()
      elif req.type == STOP:
         self._fsm.stop()
      elif req.type == ROT:
         self._fsm.rotate()
      else:
         raise ValueError('could not handle request type: %f' % req.type)


   # underscore prefixed methods are 
   # called internally from state machine

   def _error(self):
      raise StateMachineError

   def _broadcast(self):
      self._sui.send(self._fsm._state)

   def _save_power_activity(self):
      print 'save_power_activity'
      self.activity.cancel_and_join()
      self.activity = PowerSaveActivity(self._core)
      self.activity.start()

   def _takeoff_activity(self):
      print 'takeoff_activity'
      self.activity.cancel_and_join()
      self.activity = TakeoffActivity(self._fsm, self._core, self._arg)
      self.activity.start()

   def _land_activity(self):
      print 'land_activity'
      self.activity.cancel_and_join()
      self.activity = LandActivity(self._fsm, self._core)
      self.activity.start()

   def _move_activity(self):
      print 'move_activity'
      self.activity.cancel_and_join()
      self.activity = MoveActivity(self._fsm, self._core, self._arg)
      self.activity.start()

   def _stop_activity(self):
      print 'stop_activity'
      self.activity.cancel_and_join()
      self.activity = StopActivity(self._fsm, self._core)
      self.activity.start()

   def _rotate(self):
      print 'rotate'

