
from pymavlink.mavlinkv10 import *
from pymavlink.mavutil import mavserial

from mavlink_source import MAVLinkSource
from gendisp import GenDisp
from opcd_interface import OPCD_Interface
from threading import Thread
from monitor_data_pb2 import CoreMonData

from time import sleep
import re

class ParamHandler(Thread):

   def __init__(self, dispatcher):
      Thread.__init__(self)
      self.dispatcher = dispatcher
      self.opcd_interface = OPCD_Interface('mavlink')
      self.param_map = {}
      list = self.opcd_interface.get('')
      c = 0
      type_map = {float: 9, long: 6}
      for name, val in list:
         try:
            type = type_map[val.__class__]
            self.param_map[c] = name, type
            c += 1
         except:
            # string
            pass
   
   def run(self):
      for e in self.dispatcher.generator('PARAM_'):
         if e.get_type() == 'PARAM_REQUEST_LIST':
            print e
            list = self.opcd_interface.get('')
            for index, (name, type) in self.param_map.items():
               try:
                  val = self.opcd_interface.get(name)
                  name_short = re.sub('(?P<foo>\w)\w*\.', '\g<foo>.', name)
                  name_short = re.sub('_', '-', name_short)
                  name_short = re.sub('\.', '_', name_short)
                  mavio.mav.param_value_send(name_short, val, type, len(self.param_map), index)
                  print len(self.param_map), index, name_short, val, type
                  sleep(0.1)
               except Exception, ex:
                  print str(ex)
         elif e.get_type() == 'PARAM_REQUEST_READ':
            print e
            index = e.param_index
            name, type = self.param_map[index];
            try:
               val = self.opcd_interface.get(name)
               name_short = re.sub('(?P<foo>\w)\w*\.', '\g<foo>.', name)
               name_short = re.sub('_', '-', name_short)
               name_short = re.sub('\.', '_', name_short)
               sleep(0.3)
               mavio.mav.param_value_send(name_short, val, type, len(self.param_map), index)
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


from serial import Serial

port = Serial('/dev/ttyUSB1', 9600, timeout = None,
              bytesize = 8, parity = 'N', stopbits = 1,
              dsrdtr = False, 
              rtscts = False, xonxoff = False)


mavio = mavserial(port, source_system = 0x01)

source = MAVLinkSource(mavio)
dispatcher = GenDisp(source)

param_handler = ParamHandler(dispatcher)
param_handler.start()

deadbeef_handler = DeadbeefHandler(dispatcher)
deadbeef_handler.start()

dispatcher.start()

import time
from scl import generate_map

socket = generate_map('mavlink')['core_mon']


mon = CoreMonData()

def mon_read():
   while True:
      str = socket.recv()
      mon.ParseFromString(str)

Thread(target = mon_read).start()

flags = 0
while True:
   flags += 1
   if flags == 256:
      flags = 0
   
   time_ms = int(time.time() / 10)
   mavio.mav.attitude_send(time_ms, mon.roll, mon.pitch, mon.yaw, mon.roll_speed, mon.pitch_speed, mon.yaw_speed)
   mavio.mav.heartbeat_send(MAV_TYPE_QUADROTOR, MAV_AUTOPILOT_GENERIC, flags, 0, MAV_STATE_ACTIVE)
   time.sleep(0.05)
