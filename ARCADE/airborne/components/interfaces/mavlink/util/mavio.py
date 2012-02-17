
'''
mavlink IO functions

Copyright Tobias Simon 2012
Released under GNU GPL version 3 or later
'''

import mavlinkv10 as mavlink
import serial
import time
import random
import socket


class MAVIO:
    
   def __init__(self, source_system):
      self.mav = mavlink.MAVLink(self, srcSystem = source_system)
      self.mav.robust_parsing = True


class MAVIO_Serial(MAVIO):
    
   def __init__(self, device, baud, source_system):
      MAVIO.__init__(self, source_system)
      self.device = device
      self.baud = baud
      self.port = serial.Serial(self.device, self.baud,
         timeout = None, bytesize = serial.EIGHTBITS,
         parity = serial.PARITY_NONE, stopbits = serial.STOPBITS_ONE,
         dsrdtr = False, rtscts = False, xonxoff = False,
         writeTimeout = None)
      self.port.setDTR(False)

   def read(self):
      while True:
         data = self.port.read(1)
         msg = self.mav.parse_char(data)
         if msg is not None:
            return msg

   def write(self, data):
      self.port.write(data)


class MAVIO_UDP(MAVIO):
    
   def __init__(self, address, port, source_system):
      MAVIO.__init__(self, source_system)
      self.address = address
      self.port = port
      self.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

   def read(self):
      while True:
         data = self.socket.recvfrom(1)
         msg = self.mav.parse_char(data)
         if msg is not None:
            return msg

   def write(self, data):
      self.socket.sendto(data, (self.address, self.port))

