#!/usr/bin/env python


#
# ICARUS
# responsibilities:
# - takes care that the UAV system is always in a valid state by using the flight state machine
# - accepts or rejects user commands
# - performs emergency landing if battery voltage is too low
# - performs flight distance estimation based on speed and estimated flight time
# - learns possible landing spots and navigates there if the battery goes low
# - publishes state updates via SCL
#


from core_pb2 import POS_YAW
from math import atan2
from time import sleep
from icarus_pb2 import TAKEOFF, LAND, MOVE, STOP, ROT
from protocols.icarus_server import ICARUS_Exception
from activities.activities import DummyActivity, TakeoffActivity, LandActivity, StopActivity
from activities.move import MoveActivity
from flight_sm import flight_sm, flight_Hovering, flight_Moving, flight_Stopping
from util.geomath import bearing
from mtputil import *
from named_daemon import daemonize
from scl import generate_map
from protocols.icarus_server import ICARUS_Server
from core_interface import CoreInterface
from protocols.state_emitter import StateEmitter
from event_handler import EventHandler
from protocols.powerman import PowerMan


class OperationRangeEstimate:

   def __init__(self, speed)
      self.speed = speed

   def estimate(ah_remaining, ampere_rate):
      time = ah_remaining / ampere_rate
      return = self.speed * time


class ICARUS:


   def __init__(self, name):
      self.fsm = flight_sm(self)
      self.landing_spots = []
      sockets = generate_map(name)
      self.core = CoreInterface(sockets['core'], sockets['mon'])
      self.state_emitter = StateEmitter(sockets['hlsm'])
      self.powerman = PowerMan(sockets['power_ctrl'], sockets['power_mon'])
      start_daemon_thread(self.power_state_monitor)
      start_daemon_thread(self.state_time_monitor)
      self.activity = DummyActivity()
      self.activity.start()
      self.icarus_srv = ICARUS_Server(sockets['ctrl'], self)
      self.icarus_srv.start()
      self.flight_time = 0
      self.icarus_takeover = False
      self.emergency_land = False


   def state_time_monitor(self):
      while True:
         if self.fsm._state != flight_Standing:
            # count the time for "in-the-air-states":
            self.flight_time += 1
         else:
            # use system uptime here:
            up_file = open("/proc/uptime", "r")
            up_list = up_file.read().split()
            self.uptime = int(up_list[0])
            up_file.close()
         sleep(1)


   def emergency_landing(self):
       # we don't care about the system's state:
       # just try to stop and land it!
       try:
          self.fsm.stop()
       except:
          pass
       try:
          self.fsm.land()
       except:
          pass
       # after emergency landing, the interface will stay locked
       # and power circruitry will go offline


   def power_state_monitor(self):
      while True:
         self.power_state = self.powerman.read()
         if self.power_state.crtical:
            self.icarus_takeover = True
            if not self.emergency_land:
               self.emergency_landing()



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


   # called by ICARUS protocol driver
   def handle(self, req):
      if self.icarus_takeover:
         raise ICARUS_Exception('interface disabled due to ICARUS emergency take-over')
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


   # underscore prefixed methods are 
   # called internally from state machine


   def _error(self):
      raise ICARUS_Exception('flight state machine error')


   def _broadcast(self):
      self.state_emitter.send(self.fsm._state)


   def _save_power_activity(self):
      print 'save_power'
      self.powerman.stand_mode()


   def _takeoff_activity(self):
      print 'takeoff_activity'
      self.landing_spots.append((self.core.mon.x, self.core.mon.y))
      self.activity.cancel_and_join()
      self.powerman.flight_mode()
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



def main(name):
   ICARUS()   
   await_signal()


main('icarus')
daemonize('icarus', main)

