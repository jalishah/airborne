
import time
import sys

from scl import generate_map
from monitor_data_pb2 import CoreMonData
from pymavlink.mavlinkv10 import *
from pymavlink.mavutil import mavudp
from mavlink_util import ControlSensorBits

csb = ControlSensorBits()
mav_udp = mavudp('141.24.211.33:14550', False)
link = MAVLink(mav_udp)

socket = generate_map('mavlink')['core_mon']
mon = CoreMonData()

onboard_control_sensors_present = csb.bits(['GYRO_3D', 'ACC_3D', 'MAG_3D', 'PRESSURE_ABS', 'GPS', 'ANGLE_RATE_CONTROL', 'ATTITUDE_CTRL', 'YAW_CTRL', 'ALTITUDE_CTRL', 'XY_CTRL', 'MOTOR_CTRL'])
onboard_control_sensors_enabled = onboard_control_sensors_present
onboard_control_sensors_health = onboard_control_sensors_present
load = 0.5
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

def gps_add_meters(lat, lon, dx, dy):
   delta_lon = dx / (111320 * cos(lat))
   delta_lat = dy / 110540
   new_lon = lon + delta_lon
   new_lat = lat + delta_lat
   return new_lat, new_lon


while True:
   str = socket.recv()
   mon.ParseFromString(str)
   time_ms = int(time.time() / 10)
   flags = MAV_MODE_FLAG_AUTO_ENABLED | MAV_MODE_FLAG_STABILIZE_ENABLED | MAV_MODE_FLAG_SAFETY_ARMED
   link.heartbeat_send(MAV_TYPE_QUADROTOR, MAV_AUTOPILOT_GENERIC, flags, 1, MAV_STATE_ACTIVE)
   #link.global_position_int_send(time_ms, int(50.0 * 1e7), int(10.0 * 1e7), 500 * 1000, 0, 0, 0, 0, 0)
   lat, lon = gps_add_meters(mon.gps_start_lat, mon.gps_start_lon, mon.x, mon.y);
   print lat, lon
   link.global_position_int_send(time_ms, lat, lon, 0, 0, 0, 0, 0, 0)
   link.attitude_send(time_ms, mon.roll, mon.pitch, mon.yaw, mon.roll_speed, mon.pitch_speed, mon.yaw_speed)
   
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


