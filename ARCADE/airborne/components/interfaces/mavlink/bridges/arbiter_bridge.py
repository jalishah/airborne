
from mavlinkv10 import *
from psutil import cpu_percent
from util.mavlink_util import ControlSensorBits
from threading import Thread
from time import sleep
from bridge import Bridge


class ArbiterBridge(Bridge):
   def __init__(self, socket_map, mav_iface, send_interval):
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
      load_avg = load_avg[1:] + [ cpu_percent() * 10 ]
      load = 0
      for entry in load_avg:
         load += entry
      return load / len(load_avg)

   def _send(self):
      while True:
         voltage_battery = 15 * 1000
         current_battery = 0
         battery_remaining = 100
         drop_rate_comm = 1000 * 10
         errors_comm = 0
         errors_count1 = 0
         errors_count2 = 0
         errors_count3 = 0
         errors_count4 = 0
         flags = 0
         
         mavio.mav.heartbeat_send(MAV_TYPE_QUADROTOR, MAV_AUTOPILOT_GENERIC, flags, 0, MAV_STATE_ACTIVE)
         mavio.mav.sys_status_send(sensors_present, sensors_enabled, sensors_health,
            self._load_avg(), voltage_battery, current_battery, battery_remaining, drop_rate_comm, 
            errors_comm, errors_count1, errors_count2, errors_count3, errors_count4)
         sleep(self.send_interval)

