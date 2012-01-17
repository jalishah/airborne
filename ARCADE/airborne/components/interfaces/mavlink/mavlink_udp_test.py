
import random
import time
import sys
import time

from monitor_data_pb2 import CoreMonData
from pymavlink.mavlinkv10 import *
from pymavlink.mavutil import mavudp

mav_udp = mavudp('localhost:14550', False)
link = MAVLink(mav_udp)

i = 0.0
mon = CoreMonData()
while True:
   i += 0.000001
   time_ms = int(time.time() / 10)
   flags = MAV_MODE_FLAG_AUTO_ENABLED | MAV_MODE_FLAG_STABILIZE_ENABLED | MAV_MODE_FLAG_SAFETY_ARMED
   link.heartbeat_send(MAV_TYPE_QUADROTOR, MAV_AUTOPILOT_GENERIC, flags, 1, MAV_STATE_ACTIVE)
   link.global_position_int_send(time_ms, int((50.0 + i) * 1e7), int(10.0 * 1e7), 500, 0, 0, 0, 0, 0)
   link.attitude_send(time_ms, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0)
   time.sleep(0.1)

