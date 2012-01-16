
import random
import time
import socket
from mavlinkv10 import *
from mavutil import mavudp

mav_udp = mavudp('localhost:14550', False)
link = MAVLink(mav_udp)

while True:
   time_ms = int(time.time() / 10)
   flags = MAV_MODE_FLAG_AUTO_ENABLED | MAV_MODE_FLAG_STABILIZE_ENABLED | MAV_MODE_FLAG_SAFETY_ARMED
   link.heartbeat_send(MAV_TYPE_QUADROTOR, MAV_AUTOPILOT_GENERIC, flags, 0, MAV_STATE_ACTIVE)
   link.gps_global_origin_send(10.0, 50.0, 525.0)
   link.local_position_ned_send(time_ms, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0)
   link.attitude_send(time_ms, random.random(), random.random(), random.random(), 1.5, 0.2, 0.1)
   time.sleep(0.1)

