
import random
import time
import sys

from scl import generate_map
from monitor_data_pb2 import CoreMonData
from pymavlink.mavlinkv10 import *
from pymavlink.mavutil import mavudp

mav_udp = mavudp('141.24.211.19:14550', False)
link = MAVLink(mav_udp)

socket = generate_map('mavlink')['core_mon']
mon = CoreMonData()

while True:
   str = socket.recv()
   mon.ParseFromString(str)
   print mon
   time_ms = int(time.time() / 10)
   flags = MAV_MODE_FLAG_AUTO_ENABLED | MAV_MODE_FLAG_STABILIZE_ENABLED | MAV_MODE_FLAG_SAFETY_ARMED
   link.heartbeat_send(MAV_TYPE_QUADROTOR, MAV_AUTOPILOT_GENERIC, flags, 0, MAV_STATE_ACTIVE)
   #link.gps_global_origin_send(10.0, 50.0, 525.0)
   link.local_position_ned_send(time_ms, mon.x, mon.y, mon.z, mon.x_speed, mon.y_speed, mon.z_speed)
   link.attitude_send(time_ms, mon.yaw, mon.pitch, mon.roll, mon.yaw_speed, mon.pitch_speed, mon.roll_speed)


