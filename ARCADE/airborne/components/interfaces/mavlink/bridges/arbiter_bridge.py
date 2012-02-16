
from mavlinkv10 import *
from psutil import cpu_percent
from util.mavlink_util import ControlSensorBits
from threading import Thread
from time import sleep
from bridge import Bridge

'''
MAVLink defines the following modes in common.xml:
MAV_MODE_FLAG_CUSTOM_MODE_ENABLED = 1 # 0b00000001 Reserved for future use.
MAV_MODE_FLAG_TEST_ENABLED = 2 # 0b00000010 system has a test mode enabled. This flag is intended for
                        # temporary system tests and should not be
                        # used for stable implementations.
MAV_MODE_FLAG_AUTO_ENABLED = 4 # 0b00000100 autonomous mode enabled, system finds its own goal
                        # positions. Guided flag can be set or not,
                        # depends on the actual implementation.
MAV_MODE_FLAG_GUIDED_ENABLED = 8 # 0b00001000 guided mode enabled, system flies MISSIONs / mission items.
MAV_MODE_FLAG_STABILIZE_ENABLED = 16 # 0b00010000 system stabilizes electronically its attitude (and
                        # optionally position). It needs however
                        # further control inputs to move around.
MAV_MODE_FLAG_HIL_ENABLED = 32 # 0b00100000 hardware in the loop simulation. All motors / actuators are
                        # blocked, but internal software is full
                        # operational.
MAV_MODE_FLAG_MANUAL_INPUT_ENABLED = 64 # 0b01000000 remote control input is enabled.
MAV_MODE_FLAG_SAFETY_ARMED = 128 # 0b10000000 MAV safety set to armed. Motors are enabled / running / can
                        # start. Ready to fly.
'''


class ArbiterBridge(Bridge):

   def __init__(self, socket_map, mav_iface, send_interval, dispatcher, core_bridge):
      self.dispatcher = dispatcher
      self.core_bridge = core_bridge
      self.auto_mode_flags = MAV_MODE_FLAG_SAFETY_ARMED \
         | MAV_MODE_FLAG_MANUAL_INPUT_ENABLED \
         | MAV_MODE_FLAG_STABILIZE_ENABLED \
         | MAV_MODE_FLAG_GUIDED_ENABLED \
         | MAV_MODE_FLAG_AUTO_ENABLED

      Bridge.__init__(self, socket_map, mav_iface, send_interval)
      self.csb = ControlSensorBits()
      self.sensors_present = self.csb.bits(['GYRO_3D', 'ACC_3D', 'MAG_3D',
         'PRESSURE_ABS', 'GPS', 'ANGLE_RATE_CONTROL', 'ATTITUDE_CTRL',
         'YAW_CTRL', 'ALTITUDE_CTRL', 'XY_CTRL', 'MOTOR_CTRL'])
 
      self.sensors_enabled = self.sensors_present

      self.sensors_health = self.sensors_present
      self.load_avg = [ cpu_percent() * 10 ] * 60
      send_thread = Thread(target = self._send)
      send_thread.start()

   def _load_avg(self):
      self.load_avg = self.load_avg[1:] + [ cpu_percent() * 10 ]
      load = 0
      for entry in self.load_avg:
         load += entry
      return load / len(self.load_avg)

   def _send(self):
      while True:
         voltage_battery = 15 * 1000
         current_battery = 0
         battery_remaining = 100
         drop_rate_comm = int(10000.0 * self.dispatcher.loss_rate)
         errors_comm = 0
         errors_count1 = 0
         errors_count2 = 0
         errors_count3 = 0
         errors_count4 = 0
         
         if self.core_bridge.dead:
            sensors_enabled = 0
         else:
            sensors_enabled = self.sensors_enabled
         
         self.mav_iface.mavio.mav.heartbeat_send(MAV_TYPE_QUADROTOR, MAV_AUTOPILOT_GENERIC, self.auto_mode_flags, 0, MAV_STATE_ACTIVE)
         self.mav_iface.mavio.mav.sys_status_send(self.sensors_present, sensors_enabled, 0,
            self._load_avg(), voltage_battery, current_battery, battery_remaining, drop_rate_comm, 
            errors_comm, errors_count1, errors_count2, errors_count3, errors_count4)
         
         sleep(self.send_interval)

