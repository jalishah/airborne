
from mavlinkv10 import *
from mavutil import mavserial, mavudp
from psutil import cpu_percent
from scl import generate_map
from util.mavlink_util import ControlSensorBits
from util.gendisp import GenDisp
from threading import Thread
from time import time, sleep
from handlers.params import ParamHandler, DeadbeefHandler


socket_map = generate_map('mavlink')
simulate_local = True

if simulate_local:
   mavio = mavudp('localhost:14550', False, source_system = 0x01, blocking = True)
else:
   mavio = mavserial('/dev/ttyUSB1', 9600, source_system = 0x01)

dispatcher = GenDisp(mavio, True)
param_handler = ParamHandler(dispatcher)
param_handler.start()
deadbeef_handler = DeadbeefHandler(dispatcher)
deadbeef_handler.start()
dispatcher.start()


mav_iface = MAVLink_Interface(mavio)
core_bridge = CoreBridge(socket_map, mav_iface)
gps_bridge = GpsBridge(socket_map, mav_iface)

csb = ControlSensorBits()
mav_udp = mavudp('141.24.211.33:14550', False)
link = MAVLink(mav_udp)

socket = generate_map('mavlink')['core_mon']
mon = CoreMonData()

onboard_control_sensors_present = 0xAA #csb.bits(['GYRO_3D', 'ACC_3D', 'MAG_3D', 'PRESSURE_ABS', 'GPS', 'ANGLE_RATE_CONTROL', 'ATTITUDE_CTRL', 'YAW_CTRL', 'ALTITUDE_CTRL', 'XY_CTRL', 'MOTOR_CTRL'])
onboard_control_sensors_enabled = onboard_control_sensors_present
onboard_control_sensors_health = onboard_control_sensors_present
voltage_battery = 15 * 1000
current_battery = 0
battery_remaining = 100
drop_rate_comm = 1000 * 10
errors_comm = 0
errors_count1 = 0
errors_count2 = 0
errors_count3 = 0
errors_count4 = 0

load_avg = [ cpu_percent() * 10 ] * 60
flags = 0
while True:
   load_avg = load_avg[1:] + [ cpu_percent() * 10 ]
   load = 0
   for l in load_avg:
      load += l
   load /= len(load_avg)
   time_ms = int(time() / 10)
   mavio.mav.heartbeat_send(MAV_TYPE_QUADROTOR, MAV_AUTOPILOT_GENERIC, flags, 0, MAV_STATE_ACTIVE)
   mavio.mav.sys_status_send(onboard_control_sensors_present,
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

   sleep(1.0)

