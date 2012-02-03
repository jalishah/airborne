
from mavlinkv10 import *
from mavutil import mavserial, mavudp
import time
from scl import generate_map
from mavlink_source import MAVLinkSource
from opcd_interface import OPCD_Interface
from gendisp import GenDisp
from threading import Thread
from monitor_data_pb2 import CoreMonData
from gps_data_pb2 import GpsData
from math import *
from time import sleep
import re


class ParamHandler(Thread):

   def __init__(self, dispatcher):
      Thread.__init__(self)
      self.dispatcher = dispatcher
      self.opcd_interface = OPCD_Interface('mavlink')
      self.param_map = {}
      self.param_rev_map = {}
      list = self.opcd_interface.get('')
      c = 0
      type_map = {float: MAV_VAR_FLOAT, long: MAV_VAR_INT32}
      cast_map = {float: float, long: int}
      for name, val in list:
         try:
            type = type_map[val.__class__]
            self.param_map[c] = name, type, cast_map[val.__class__]
            self.param_rev_map[c] = type, name, cast_map[val.__class__]
            c += 1
         except Exception, e:
            print str(e)
   
   def run(self):
      for e in self.dispatcher.generator('PARAM_'):
         print e
         if e.get_type() == 'PARAM_REQUEST_LIST':
            list = self.opcd_interface.get('')
            for index, (name, type, cast) in self.param_map.items():
               try:
                  val = self.opcd_interface.get(name)
                  #name_short = re.sub('(?P<foo>\w)\w*\.', '\g<foo>.', name)
                  name_short = re.sub('_', '-', name)
                  name_short = re.sub('\.', '_', name_short)
                  mavio.mav.param_value_send(name_short, float(val), type, len(self.param_map), index)
               except Exception, ex:
                  print str(ex)
         elif e.get_type() == 'PARAM_REQUEST_READ':
            index = e.param_index
            name, type, cast = self.param_map[index]
            try:
               val = self.opcd_interface.get(name)
               #name_short = re.sub('(?P<foo>\w)\w*\.', '\g<foo>.', name)
               name_short = re.sub('_', '-', name)
               name_short = re.sub('\.', '_', name_short)
               mavio.mav.param_value_send(name_short, float(val), type, len(self.param_map), index)
            except Exception, ex:
               print str(ex)


class DeadbeefHandler(Thread):

   def __init__(self, dispatcher):
      Thread.__init__(self)
      self.dispatcher = dispatcher

   def run(self):
      for e in self.dispatcher.generator('BAD_DATA'):
         print 'bad data ignored'



socket_map = generate_map('mavlink')
simulate_local = False

if simulate_local:
   mavio = mavudp('localhost:14550', False, source_system = 0x01, blocking = True)
else:
   mavio = mavserial('/dev/ttyUSB1', 9600, source_system = 0x01)
source = MAVLinkSource(mavio)
dispatcher = GenDisp(source, True)

param_handler = ParamHandler(dispatcher)
param_handler.start()

deadbeef_handler = DeadbeefHandler(dispatcher)
deadbeef_handler.start()

dispatcher.start()


class Bridge:

   '''
   A bridge connects a data source to a data sink (usually via SCL)
   and implements the required data conversion in order to send it to
   the MAVLink peer.
   '''

   def __init__(self, socket_map, mav_iface):
      self.socket_map = socket_map
      self.mav_iface = mav_iface


class GpsBridge(Bridge):
   
   def __init__(self, socket_map, mav_iface):
      Bridge.__init__(self, socket_map, mav_iface)
      recv_thread = Thread(target = self._receive)
      send_thread = Thread(target = self._send)
      recv_thread.start()
      send_thread.start()

   def _receive(self):
      socket = self.socket_map['gps']
      gps = GpsData()
      while True:
         str = socket.recv()
         gps.ParseFromString(str)
         self.gps = gps

   def _send(self):
      while True:
         try:
            gps = self.gps
            self.mav_iface.send_gps_position(gps.fix, gps.lon, gps.lat, gps.alt, gps.hdop, gps.vdop, gps.speed, gps.course, len(gps.satinfo)):
            sleep(1.0)
         except:
            pass


class CoreBridge(Bridge):
    
   # def _gps_add_meters(self, lat, lon, dx, dy):
   #   delta_lon = dx / (111320 * cos(lat))
   #   delta_lat = dy / 110540
   #   new_lon = lon + delta_lon
   #   new_lat = lat + delta_lat
   #   return new_lat, new_lon


   #   lat, lon = self._gps_add_meters(mon.gps_start_lat, mon.gps_start_lon, mon.x, mon.y)

  
   def __init__(self, socket_map, mav_iface):
      Bridge.__init__(self, socket_map, mav_iface)
      recv_thread = Thread(target = self._receive)
      send_thread = Thread(target = self._send)
      recv_thread.start()
      send_thread.start()

   def _receive(self):
      mon = CoreMonData()
      socket = self.socket_map['core_mon']
      while True:
         str = socket.recv()
         mon.ParseFromString(str)
         self.mon = mon

   def _send(self):
      while True:
         try:
            mon = self.mon
            self.mav_iface.send_local_position(mon.x, mon.y, mon.z, mon.x_speed, mon.y_speed, mon.z_speed)
            self.mav_iface.send_attitude(mon.roll, mon.pitch, mon.yaw, mon.roll_speed, mon.pitch_speed, mon.yaw_speed)
            sleep(0.3)
         except:
            pass


class MAVLink_Interface:

   def __init__(self, mavio):
      self.mavio = mavio
   
   def _uptime(self):
      uptime_file = open("/proc/uptime")
      uptime_list = uptime_file.read().split()
      uptime_file.close()
      return float(uptime_list[0])

   def _uptime_ms(self):
      return int(self._uptime() * 1000.0)

   def _uptime_us(self):
      return long(self._uptime() * 1000000.0)

   def _time_ms(self):
      print int(time.time() * 1000.0)
      return int(time.time() * 1000.0)

   def _time_us(self):
      return long(time.time() * 1000000.0)

   def send_local_position(self, x, y, z, vx, vy, vz):
      '''
      filtered local position (e.g. fused GPS and accelerometers).
      Coordinate frame is right-handed, Z-axis down (aeronautical frame, NED / north-east-down convention)
      '''
      self.mavio.mav.local_position_ned_send(self._uptime_ms(),
         float(x), # X Position in m
         float(y), # Y Position in m
         float(z), # Z Position in m
         float(vx), # X Speed in m/s
         float(vy), # Y Speed in m/s 
         float(vz)) # Z Speed in m/s

   def send_attitude(self, roll, pitch, yaw, roll_speed, pitch_speed, yaw_speed):
      self.mavio.mav.attitude_send(self._uptime_ms(),
         float(roll), # Roll angle (rad)
         float(pitch), # Pitch angle (rad)
         float(yaw), # Yaw angle (rad)
         float(roll_speed), # Roll angular speed (rad/s)
         float(pitch_speed), # Pitch angular speed (rad/s)
         float(yaw_speed)) # Yaw angular speed (rad/s)

   def send_gps_position(self, fix_type, lon_deg, lat_deg, alt_m, hdop_m = None, vdop_m = None, speed_m_s = None, course_deg = None, sats_visible = None):
      params = [self._time_us(), # Timestamp (microseconds since UNIX epoch or microseconds since system boot)
         fix_type, # 0-1: no fix, 2: 2D fix, 3: 3D fix. Some applications will not use the value of this field unless it is at least two, so always correctly fill in the fix.
         long(lat_deg * 1.0e7), # Latitude in 1E7 degrees
         long(lon_deg * 1.0e7), # Longitude in 1E7 degrees
         int(alt_m * 1.0e3) # Altitude in 1E3 meters (millimeters) above MSL
      ]
      if hdop_m:
         params.append(int(hdop_m * 100.0)) # GPS HDOP horizontal dilution of position in cm (m*100).
      else:
         params.append(65535) # If unknown, set to: 65535
      if vdop_m:
         params.append(int(vdop_m * 100.0)) # GPS VDOP horizontal dilution of position in cm (m*100). If unknown, set to: 65535
      else:
         params.append(65535) # If unknown, set to: 65535
      if hdop_m:
         params.append(int(speed_m_s * 100.0)) # GPS ground speed (m/s * 100).
      else:
         params.append(65535) # If unknown, set to: 65535
      if course_deg:
         params.append(int(course_deg * 100.0)) # Course over ground (NOT heading, but direction of movement) in degrees * 100, 0.0..359.99 degrees.
      else:
         params.append(65535) # If unknown, set to: 65535
      if sats_visible:
         params.append(sats_visible) # Number of satellites visible.
      else:
         params.append(255) # If unknown, set to 255
      self.mavio.mav.gps_raw_int_send(*params)


mav_iface = MAVLink_Interface(mavio)
CoreBridge(socket_map, mav_iface)
GpsBridge(socket_map, mav_iface)

flags = 0
while True:
   time_ms = int(time.time() / 10)
   mavio.mav.heartbeat_send(MAV_TYPE_QUADROTOR, MAV_AUTOPILOT_GENERIC, flags, 0, MAV_STATE_ACTIVE)
   time.sleep(1.0)

