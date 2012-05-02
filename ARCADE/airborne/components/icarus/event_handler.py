
#
# the flight manager is the central component of ICARUS
# it
# - takes care that the UAV system is always in a valid state by using the flight state machine
# - accepts or rejects user commands
# - publishes state updates via SCL
# - manages acoustic state and error signaling
#


from core_pb2 import POS_YAW, SPEED_YAW
from math import atan2
from time import sleep
from icarus_pb2 import TAKEOFF, LAND, MOVE, STOP, ROT
from protocols.icarus_server import ICARUS_Exception
from activities.activities import DummyActivity, TakeoffActivity, LandActivity, StopActivity
from activities.move import MoveActivity
from flight_sm import flight_sm, flight_Hovering, flight_Moving, flight_Stopping
from util.geomath import bearing


class EventHandler:


   def __init__(self, core, state_emitter):
      self.core = core
      self.state_emitter = state_emitter
      self.fsm = flight_sm(self)
      self.landing_spots = []
      self.activity = DummyActivity()
      self.activity.start()


   def poi_thread(self):
      '''
      This method/thread is responsible for updating the yaw setpoint of the system.
      The code is only executed when the system is in a valid state.
      '''
      prev_yaw = None
      min_rot_ground_alt = 2.0
      valid_states = [flight_Hovering, flight_Moving, flight_Stopping]
      while True:
         if self.fsm._state != flight_Landing and self.monitor.data.z_ground > min_rot_ground_alt:
            print 'system is able to rotate'
            try:
               if isinstance(self.yaw_target, float):
                  print 'fixed yaw mode'
                  yaw = self.yaw
               else:
                  poi_x = self.yaw_target[0]
                  poi_y = self.yaw_target[1]
                  print 'POI mode, x =', poi_x, ' y =', poi_y
                  yaw = atan2(self.core.mon.y - poi_y, self.core.mon.x - poi_x)
               if prev_yaw != yaw:
                  print 'setting yaw to:', yaw
                  self.core.set_ctrl_param(POS_YAW, yaw)
               prev_yaw = yaw
            except AttributeError:
               pass
         sleep(1)


   # called externally from ICARUS protocol driver
   def handle(self, req):
      self.arg = req
      if req.type == TAKEOFF:
         self.fsm.takeoff()
      elif req.type == LAND:
         self.fsm.land()
      elif req.type == MOVE:
         self.fsm.move()
      elif req.type == STOP:
         self.fsm.stop()
      elif req.type == ROT:
         if len(self.arg.pos) == 1:
            self.yaw_target = self.arg.pos[0]
         else:
            self.yaw_target = self.arg.pos[0], self.arg.pos[1]
      else:
         raise ValueError('could not handle request type: %f' % req.type)



   # underscore prefixed methods are 
   # called internally from state machine


   def _error(self):
      raise ICARUS_Exception(-1, 'flight state machine error')


   def _broadcast(self):
      self.state_emitter.send(self.fsm._state)


   def _save_power_activity(self):
      print 'save_power_activity'
      self.activity.cancel_and_join()
      self.activity = PowerSaveActivity(self.core)
      self.activity.start()


   def _takeoff_activity(self):
      print 'takeoff_activity'
      self.landing_spots.append((self.core.mon.x, self.core.mon.y))
      self.activity.cancel_and_join()
      self.activity = TakeoffActivity(self.fsm, self.core, self.arg)
      self.activity.start()


   def _land_activity(self):
      print 'land_activity'
      self.activity.cancel_and_join()
      self.activity = LandActivity(self.fsm, self.core, self.core.mon)
      self.activity.start()


   def _move_activity(self):
      print 'move_activity'
      self.activity.cancel_and_join()
      self.activity = MoveActivity(self.fsm, self.core, self.arg)
      self.activity.start()


   def _stop_activity(self):
      print 'stop_activity'
      self.activity.cancel_and_join()
      self.activity = StopActivity(self.fsm, self.core)
      self.activity.start()

