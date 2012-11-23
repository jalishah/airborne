#!/usr/bin/env python

from threading import Thread, Timer

from misc import *
from cal_pb2 import *
from scl import generate_map
from core_interface import CoreInterface


class CalServer:

   def __init__(self, cal_socket):
      self.cal_socket = cal_socket
      self.request_thread = start_daemon_thread(self.cal_reader)

   def cal_reader(self):
      while True:
         cal_data = CalData()
         try:
            cal_data.ParseFromString(self.cal_socket.recv())
         except:
            sleep(1)
            continue
         print cal_data



def main(name):
   map = generate_map(name)
   core = CoreInterface(map['ctrl'], None)
   # configure core to operate in calibration mode:
   core.mode_cal()
   cal_socket = map['cal']
   CalServer()
   await_signal()
   # configure core to operate in normal mode:
   core.mode_normal()



main('cal_server')
#daemonize('cal_server', main)

