
'''
mavlink IO functions

Copyright Tobias Simon 2012
Released under GNU GPL version 3 or later
'''

import mavlinkv10 as mavlink
import serial


class MAVIO:
    
   def __init__(self, address, source_system):
      self.mav = mavlink.MAVLink(self, srcSystem = source_system)
      self.mav.robust_parsing = True


class MAVIO_Serial(MAVIO):
    
   def __init__(self, device, baud, source_system):
      self.device = device
      self.baud = baud
      self.port = serial.Serial(self.device, self.baud, timeout = None, dsrdtr = False, rtscts = False, xonxoff = False)
      MAVIO.__init__(self, source_system)

   def read(self):
      while True:
         data = self.port.read(1)
         msg = self.mav.parse_char(data)
         if msg is not None:
            return msg

   def write(self, data):
      self.port.write(data) # NOTE: this call is already thread-safe!

