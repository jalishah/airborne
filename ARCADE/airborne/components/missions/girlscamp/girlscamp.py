#!/usr/bin/env python

import sys
import zmq
from imms_sensor_data_pb2 import SensorData
from scl import generate_map
from time import sleep, time
from threading import Thread, Event
from core_pb2 import MonData
from icarus_interface import ICARUS_Client, ICARUS_MissionFactory
import tnts
import cenn
import tam_ctrl
import tobi
import jan


class MonReader(Thread):

   def __init__(self, socket):
      Thread.__init__(self)
      self.daemon = True
      self.socket = socket
      self.event = Event()

   def run(self):
      while True:
         data = MonData()
         data.ParseFromString(self.socket.recv())
         self.data = data
         self.event.set()


class IMMS_Reader(Thread):

   def __init__(self, socket):
      Thread.__init__(self)
      self.daemon = True
      self.socket = socket
      self.event = Event()

   def run(self):
      while True:
         data = SensorData()
         data.ParseFromString(self.socket.recv())
         self.data = data
         print data
         self.event.set()



sm = generate_map('girlscamp')
print sm
mon_reader = MonReader(sm['core_data'])
imms_reader = IMMS_Reader(sm['imms_data'])
mon_reader.start()
imms_reader.start()
_client = ICARUS_Client(sm['ctrl'])
i = ICARUS_MissionFactory()


def request(item):
   try:
      _client.execute(item)
   except Exception, e:
      print e


class Ctrl:

   def __init__(self):
      self.pos = [(0,0), (10, 0), (10, 10), (0, 10), (0,0)]
      self.i = -1

   def decide(self, x, y, s, t):
      self.i += 1
      return (self.pos[self.i][0], self.pos[self.i][1], True)


ctrl = tobi.Controller()

#ctrl = Ctrl()
ctrl = tnts.Controller(0, 0, 0)
#ctrl = cenn.Controller()
#ctrl = jan.Controller()
#ctrl = tam_ctrl.Controller()
#import new
#ctrl = new.Controller()
#ctrl = tobi.Controller()



if 1:
#try:
   while True:
      # read inputs:
      imms_reader.event.wait()
      imms_reader.event.clear()
      rssi = imms_reader.data.rssi
      x = mon_reader.data.x
      y = mon_reader.data.y
      
      # call decision function:
      x_dest, y_dest, change = ctrl.decide(x, y, rssi, time())
      
      # log decision:
      print x, y, rssi, x_dest, y_dest, change
      #sleep(15)
      
      # update uav setpoint:
      if change:
         request(i.move_xy(x_dest, y_dest))
#except:
#   pass

