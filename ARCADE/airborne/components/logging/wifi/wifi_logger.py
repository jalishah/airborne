#!/usr/bin/env python

import sys
import zmq
from core_pb2 import MonData
from networks_pb2 import Measurement
from scl import generate_map
from time import sleep, time
from threading import Thread, Event
from misc import daemonize


class MonReader(Thread):

   """
   reads kalman-filtered GPS positions
   """

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


class NetworksReader(Thread):

   """
   reads WiFi network information
   """

   def __init__(self, socket):
      Thread.__init__(self)
      self.daemon = True
      self.socket = socket
      self.event = Event()

   def run(self):
      while True:
         data = Measurement()
         raw_data = self.socket.recv()
         data.ParseFromString(raw_data)
         self.data = data
         self.event.set()


def main(name):
   map = generate_map(name)
   mon_reader = MonReader(map['mon'])
   networks_reader = NetworksReader(map['networks'])
   mon_reader.start()
   networks_reader.start()
   
   netfile = open('/tmp/netfile.txt', 'w')
   mon_reader.event.wait()
   while True:
      networks_reader.event.wait()
      networks_reader.event.clear()
      measure = networks_reader.data
      tstamp = time()
      netfile.write('%f; %f; %f; %f; %s; %s\n' % (tstamp, mon_reader.data.x, mon_reader.data.y, mon_reader.data.z, measure.mac, measure.rssi))
      netfile.flush()


main('wifi_logger')
daemonize('wifi_logger', main)

