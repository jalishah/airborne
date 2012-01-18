
import random
import time
import sys
import time

from monitor_data_pb2 import CoreMonData
from pymavlink.mavlinkv10 import *
from pymavlink.mavutil import mavudp


class ControlSensorBits:

   def __init__(self):
      self.cs_list = [
         'GYRO_3D',
         'ACC_3D',
         'MAG_3D',
         'PRESSURE_ABS',
         'PRESSURE_DIFF',
         'GPS',
         'OPTICAL_FLOW',
         'COMPUTER_VISION',
         'LASER_SCANNER',
         'VICON_LEICA',
         'ANGLE_RATE_CONTROL',
         'ATTITUDE_CTRL',
         'YAW_CTRL',
         'ALTITUDE_CTRL',
         'XY_CTRL',
         'MOTOR_CTRL'
      ]

   def bits(self, flag_names):
      bits = 0
      for i in range(0, len(self.cs_list)):
         if self.cs_list[i] in flag_names:
            bits |= 1 << i
      return bits

csb = ControlSensorBits()
onboard_control_sensors_present = csb.bits(['GYRO_3D', 'ACC_3D', 'MAG_3D', 'PRESSURE_ABS', 'GPS', 'ANGLE_RATE_CONTROL', 'ATTITUDE_CTRL', 'YAW_CTRL', 'ALTITUDE_CTRL', 'XY_CTRL', 'MOTOR_CTRL'])
onboard_control_sensors_enabled = onboard_control_sensors_present
onboard_control_sensors_health = onboard_control_sensors_present
load = 0.5

mav_udp = mavudp('141.24.211.33:14550', False)
link = MAVLink(mav_udp, 1)
voltage_battery = 15 * 1000
current_battery = -1
battery_remaining = 10
drop_rate_comm = 0.5
errors_comm = 0
errors_count1 = 0
errors_count2 = 0
errors_count3 = 0
errors_count4 = 0

from math import *

def convert(lat, lon, dx, dy):
   delta_lon = dx / (111320 * cos(lat))
   delta_lat = dy / 110540
   new_lon = lon + delta_lon
   new_lat = lat + delta_lat
   return new_lat, new_lon

lat = 51.0
lon = 13.0
d = 0.0
mon = CoreMonData()
while True:
   lat, lon = convert(lat, lon, -0.1, -0.1)
   time_ms = int(time.time() / 10)
   flags = MAV_MODE_FLAG_AUTO_ENABLED | MAV_MODE_FLAG_STABILIZE_ENABLED | MAV_MODE_FLAG_SAFETY_ARMED
   link.heartbeat_send(MAV_TYPE_QUADROTOR, MAV_AUTOPILOT_GENERIC, flags, 0, MAV_STATE_ACTIVE)
   link.global_position_int_send(time_ms, int((lat) * 1e7), int(lon * 1e7), 500, 0, 0, 0, 0, 0)
   link.attitude_send(time_ms, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0)
   
   link.sys_status_send(onboard_control_sensors_present,
                        onboard_control_sensors_enabled,
                        onboard_control_sensors_health,
                        load, 
                        voltage_battery, 
                        current_battery, 
                        battery_remaining, 
                        drop_rate_comm, 
                        errors_comm, 
                        errors_count1,
                        errors_count2, 
                        errors_count3, 
                        errors_count4)
   time.sleep(0.1)

