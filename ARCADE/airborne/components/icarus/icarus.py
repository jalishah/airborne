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
from time import sleep, time
from icarus_pb2 import TAKEOFF, LAND, MOVE, STOP, ROT
from protocols.icarus_server import ICARUS_Exception
from activities.takeoff import TakeoffActivity
from activities.land import LandActivity
from activities.move import MoveActivity
from activities.stop import StopActivity
from activities.dummy import DummyActivity
from flight_sm import flight_sm, flight_Hovering
from flight_sm import flight_Standing, flight_Moving, flight_Stopping
from util.geomath import bearing
from mtputil import *
from named_daemon import daemonize
from scl import generate_map
from protocols.icarus_server import ICARUS_Server
from core_interface import CoreInterface
from protocols.state_emitter import StateEmitter
from protocols.powerman import PowerMan
from logging import basicConfig as log_config, debug as log_debug
from logging import info as log_info, warning as log_warn, error as log_err
from logging import DEBUG
from os import sep
from paths import user_data_dir


#class OperationRangeEstimate:
#
#   def __init__(self, speed):
#      self.speed = speed
#
#   def estimate(ah_remaining, ampere_rate):
#      time_remaining = ah_remaining / ampere_rate
#      return = self.speed * time_remaining


class ICARUS:

   def __init__(self, sockets):
      logfile = user_data_dir() + os.sep + 'ICARUS.log'
      log_config(filename = logfile, level = DEBUG,
                 format = '%(asctime)s - %(levelname)s: %(message)s')
      log_info('icarus starting up')
      self.flight_time = 0
      self.icarus_takeover = False
      self.emergency_land = False
      self.return_when_signal_lost = True
      self.fsm = flight_sm(self)
      self.landing_spots = []
      self.core = CoreInterface(sockets['core'], sockets['mon'])
      self.state_emitter = StateEmitter(sockets['hlsm'])
      self.powerman = PowerMan(sockets['power_ctrl'], sockets['power_mon'])
      start_daemon_thread(self.power_state_monitor)
      start_daemon_thread(self.state_time_monitor)
      self.activity = DummyActivity()
      self.activity.start()
      self.icarus_srv = ICARUS_Server(sockets['ctrl'], self)
      self.icarus_srv.start()
      log_info('icarus up and running')


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


   def move_and_land(self, x, y):
      # try to stop and move somewhere
      try:
         self.fsm.stop()
      except:
         pass
      try:
         self.arg = IcarusReq()
         self.arg.type = MOVE
         self.arg.move_data.p0 = x
         self.arg.move_data.p1 = y
         self.fsm.move()
      except:
         pass
      try:
         self.fsm.land()
      except:
         pass


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


   def core_monitor(self):
      last_valid = time()
      self.mon_data = MonData()
      while True:
         self.core.mon_read(self.mon_data)
         if self.mon_data.signal_valid:
            last_valid = time()
         else:
            if time() - rc_timeout < last_valid:
               log_err('invalid RC signal')
               self.icarus_takeover = True


   def power_state_monitor(self):
      while True:
         self.power_state = self.powerman.read()
         if self.power_state.critical:
            log_warn('critical power state: emergency landing')
            # disable system interface and take over control:
            self.icarus_takeover = True
            if not self.emergency_land:
               self.emergency_landing()


   def rotate(self, arg):
      if len(self.arg.pos) == 1:
         self.yaw_target = self.arg.pos[0]
      else:
         self.yaw_target = self.arg.pos[0], self.arg.pos[1]


   def yaw_update_thread(self):
      '''
      This method/thread is responsible for updating the yaw setpoint of the system.
      The code is only executed when the system is in a valid state.
      '''
      prev_yaw = None
      min_rot_z_ground = 1.0
      while True:
         sleep(1)
         # when landing: setting a new rotation setpoint is not allowed:
         if self.fsm._state is flight_Landing:
            continue
         if self.monitor.data.z_ground < self.min_rot_z_ground:
            print 'system is able to rotate'
            try:
               if isinstance(self.yaw_target, float):
                  print 'fixed yaw mode'
                  yaw = self.yaw
               else:
                  poi_x = self.yaw_target[0]
                  poi_y = self.yaw_target[1]
                  print 'POI mode, x =', poi_x, ' y =', poi_y
                  yaw = atan2(self.mon_data.y - poi_y, self.mon_data.x - poi_x)
               if prev_yaw != yaw:
                  print 'setting yaw to:', yaw
                  self.core.set_ctrl_param(POS_YAW, yaw)
               prev_yaw = yaw
            except AttributeError:
               pass


   # called by ICARUS protocol driver:
   def handle(self, req):
      if self.icarus_takeover:
         msg = 'request rejected due to emergency take-over'
         log_err(msg)
         raise ICARUS_Exception(msg)
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
         self.rotate(arg)

   # the following _prefixed methods are 
   # called internally from state machine


   def _error(self):
      msg = 'flight state machine error'
      log_err(msg)
      raise ICARUS_Exception(msg)


   def _broadcast(self):
      log_info('new state: %d' % self.fsm_state)
      self.state_emitter.send(self.fsm._state)


   def _save_power_activity(self):
      log_info('standing')
      self.powerman.stand_mode()


   def _takeoff_activity(self):
      log_info('taking off')
      self.landing_spots.append((self.mon_data.x, self.mon_data.y))
      self.activity.cancel_and_join()
      self.powerman.flight_mode()
      self.yaw_setpoint = self.mon.yaw
      self.activity = TakeoffActivity(self.fsm, self.core, self.arg)
      self.activity.start()


   def _land_activity(self):
      log_info('landing')
      self.activity.cancel_and_join()
      self.activity = LandActivity(self.fsm, self.core, self.mon_data)
      self.activity.start()


   def _move_activity(self):
      log_info('moving')
      self.activity.cancel_and_join()
      self.activity = MoveActivity(self.fsm, self.core, self.arg)
      self.activity.start()


   def _stop_activity(self):
      log_info('stopping')
      self.activity.cancel_and_join()
      self.activity = StopActivity(self.fsm, self.core)
      self.activity.start()



def main(name):
   sockets = generate_map(name)
   ICARUS(sockets)
   await_signal()


main('icarus')
#daemonize('icarus', main)

