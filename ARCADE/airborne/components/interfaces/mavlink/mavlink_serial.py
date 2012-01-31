
from pymavlink.mavlinkv10 import *
from pymavlink.mavutil import mavserial

from mavlink_source import MAVLinkSource
from gendisp import GenDisp
from opcd_interface import OPCD_Interface
from threading import Thread


class MAVIO:

   def __init__(self):
      self.list = [MAVLink_param_request_list_message(1, 0),
                   MAVLink_param_request_read_message(1, 0, 'my_param', -1)]
      self.i = 0

   def recv_msg(self):
      if self.i == len(self.list):
         return
      msg = self.list[self.i]
      self.i += 1
      return msg


class ParamHandler(Thread):

   def __init__(self, dispatcher):
      Thread.__init__(self)
      self.dispatcher = dispatcher

   def run(self):
      for e in self.dispatcher.generator('PARAM_'):
         if e.get_type() == 'PARAM_REQUEST_LIST':
            print 'list'
         elif e.get_type() == 'PARAM_REQUEST_READ':
            print 'read'


opcd_interface = OPCD_Interface('mavlink')
mavio = MAVIO() #mavserial('/dev/ttyO3', 9600)

#opcd_interface.get(key)
#opcd_interface.set(key, val)
#opcd_interface.persist()

source = MAVLinkSource(mavio)
dispatcher = GenDisp(source)

param_handler = ParamHandler(dispatcher)
param_handler.start()
dispatcher.start()

