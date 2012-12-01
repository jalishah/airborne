#!/usr/bin/env python

from threading import Thread, Timer

from misc import *
from cal_pb2 import *
from scl import generate_map
from core_interface import CoreInterface
import socket
import struct
from time import sleep
from opcd_interface import OPCD_Interface


class CalServer:

   def __init__(self, cal_socket, opcd):
      self.cal_socket = cal_socket
      self.opcd = opcd
      self.cal_reader_thread = start_daemon_thread(self.cal_reader)
      self.cal_tcp_handler_thread = start_daemon_thread(self.cal_tcp_handler)

   def cal_reader(self):
      self.cal_data = CalData()
      while True:
         try:
            self.cal_data.ParseFromString(self.cal_socket.recv())
         except Exception, e:
            print e
            sleep(1)
            continue


   def cal_tcp_handler(self):
      while True:
         s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
         s.bind(('', 10003))
         s.listen(1)
         conn, addr = s.accept()
         print 'new connection'
         ACC_SCALE = 1000.0
         MAG_SCALE = 10.0
         while True:
            try:
               t = conn.recv(1)
               if t == 'v':
                  conn.send('ARCADE Calibration Server\r\n')
               elif t == 'b':
                  count = ord(conn.recv(1))
                  for _ in range(count):
                     raw = [self.cal_data.ax * ACC_SCALE, self.cal_data.ay * ACC_SCALE, self.cal_data.az * ACC_SCALE,
                            0,                0,                0, 
                            self.cal_data.mx * MAG_SCALE, self.cal_data.my * MAG_SCALE, self.cal_data.mz * MAG_SCALE]
                     for r in raw:
                        conn.send(struct.pack('h', int(r)))
                     conn.send('\r\n')
                     sleep(0.01)
               elif t == 'c':
                  dim = {0: 'x', 1: 'y', 2: 'z'}
                  acc_biases = map(lambda x: x / ACC_SCALE, struct.unpack('<hhh', conn.recv(3 * 2)))
                  mag_biases = map(lambda x: x / MAG_SCALE, struct.unpack('<hhh', conn.recv(3 * 2)))
                  acc_scales = struct.unpack('<fff', conn.recv(3 * 4)) # TODO: scaling?
                  mag_scales = struct.unpack('<fff', conn.recv(3 * 4)) # ?
                  print acc_biases, acc_scales, mag_biases, mag_scales
                  for i in range(3):
                     self.opcd.set('core.cal.acc_bias_%d' % dim[i], acc_biases[i])
                     self.opcd.set('core.cal.mag_bias_%d' % dim[i], mag_biases[i])
                     self.opcd.set('core.cal.acc_scale_%d' % dim[i], acc_scales[i])
                     self.opcd.set('core.cal.mag_scale_%d' % dim[i], mag_scales[i])
            except:
               print 'connection closed'
         conn.close()
         s.close()


def main(name):
   map = generate_map(name)
   core = CoreInterface(map['core_ctrl'], None)
   # configure core to operate in calibration mode:
   core.mode_cal()
   cal_socket = map['cal']
   opcd = OPCD_Interface(map['opcd_opcd'])
   CalServer(cal_socket, opcd)
   await_signal()
   # configure core to operate in normal mode:
   core.mode_normal()



main('cal_server')
#daemonize('cal_server', main)


