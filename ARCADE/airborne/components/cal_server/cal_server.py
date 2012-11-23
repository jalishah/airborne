#!/usr/bin/env python

from threading import Thread, Timer

from misc import *
from cal_pb2 import *
from scl import generate_map
from core_interface import CoreInterface
import socket
import struct
from time import sleep


class CalServer:

   def __init__(self, cal_socket):
      self.cal_socket = cal_socket
      self.cal_reader_thread = start_daemon_thread(self.cal_reader)
      self.cal_tcp_handler_thread = start_daemon_thread(self.cal_tcp_handler)

   def cal_reader(self):
      while True:
         self.cal_data = CalData()
         try:
            self.cal_data.ParseFromString(self.cal_socket.recv())
         except:
            sleep(1)
            continue
         print cal_data


   def cal_tcp_handler(self):
      while True:
         s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
         s.bind(('', 10000))
         s.listen(1)
         conn, addr = s.accept()
         while True:
            try:
               t = conn.recv(1)
               if t == 'v':
                  conn.send('ARCADE Calibration Server\r\n')
               elif t == 'b':
                  count = ord(conn.recv(1))
                  for _ in range(count):
                     raw = [self.cal_data.ax, self.cal_data.ay, self.cal_data.az,
                            0,                0,                0, 
                            self.cal_data.mx, self.cal_data.my, self.cal_data.mz]
                     for r in raw:
                        conn.send(struct.pack('h', r))
                     conn.send('\r\n')
                     sleep(0.05)
            except:
               print 'connection closed'
         conn.close()
         s.close()


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


