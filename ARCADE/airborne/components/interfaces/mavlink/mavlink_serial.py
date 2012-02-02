
from mavlinkv10 import *
from mavutil import mavserial, mavudp
import time
from scl import generate_map
from mavlink_source import MAVLinkSource
from gendisp import GenDisp
from opcd_interface import OPCD_Interface
from threading import Thread
from monitor_data_pb2 import CoreMonData
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
         if e.get_type() == 'PARAM_REQUEST_LIST':
            print e
            list = self.opcd_interface.get('')
            for index, (name, type, cast) in self.param_map.items():
               try:
                  val = self.opcd_interface.get(name)
                  #name_short = re.sub('(?P<foo>\w)\w*\.', '\g<foo>.', name)
                  name_short = re.sub('_', '-', name)
                  name_short = re.sub('\.', '_', name_short)
                  mavio.mav.param_value_send(name_short, cast(val), type, len(self.param_map), index)
                  print len(self.param_map), index, name_short, val, type
               except Exception, ex:
                  print str(ex)
         elif e.get_type() == 'PARAM_REQUEST_READ':
            print e
            index = e.param_index
            name, type, cast = self.param_map[index]
            try:
               val = self.opcd_interface.get(name)
               #name_short = re.sub('(?P<foo>\w)\w*\.', '\g<foo>.', name)
               name_short = re.sub('_', '-', name)
               name_short = re.sub('\.', '_', name_short)
               mavio.mav.param_value_send(name_short, cast(val), type, len(self.param_map), index)
               print len(self.param_map), index, name_short, val, type
            except Exception, ex:
               print str(ex)
         else:
            print e


class DeadbeefHandler(Thread):

   def __init__(self, dispatcher):
      Thread.__init__(self)
      self.dispatcher = dispatcher

   def run(self):
      for e in self.dispatcher.generator('BAD_DATA'):
         print 'bad data ignored'


def gps_add_meters(lat, lon, dx, dy):
   delta_lon = dx / (111320 * cos(lat))
   delta_lat = dy / 110540
   new_lon = lon + delta_lon
   new_lat = lat + delta_lat
   return new_lat, new_lon


simulate_local = True

if simulate_local:
   mavio = mavudp('localhost:1234', False)
else:
   mavio = mavserial('/dev/ttyACM0', 115200, source_system = 0x01)
source = MAVLinkSource(mavio)
dispatcher = GenDisp(source)
dispatcher.start()

param_handler = ParamHandler(dispatcher)
param_handler.start()

deadbeef_handler = DeadbeefHandler(dispatcher)
deadbeef_handler.start()


def mon_read():
   socket = generate_map('mavlink')['core_mon']
   while True:
      str = socket.recv()
      mon.ParseFromString(str)


mon = CoreMonData()
if not simulate:
   Thread(target = mon_read).start()

flags = 0
while True:
   time_ms = int(time.time() / 10)
   mavio.mav.heartbeat_send(MAV_TYPE_QUADROTOR, MAV_AUTOPILOT_GENERIC, flags, 0, MAV_STATE_ACTIVE)
   lat, lon = gps_add_meters(mon.gps_start_lat, mon.gps_start_lon, mon.x, mon.y);
   mavio.mav.global_position_int_send(time_ms, lat, lon, mon.z, 0, mon.x_speed, mon.y_speed, mon.z_speed, 0)
   mavio.mav.attitude_send(time_ms, mon.roll, mon.pitch, mon.yaw, mon.roll_speed, mon.pitch_speed, mon.yaw_speed)
   time.sleep(0.2)
