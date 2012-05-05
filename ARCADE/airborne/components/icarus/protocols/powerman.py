
from power_pb2 import *
from threading import Thread
from time import sleep
from mtputil import start_daemon_thread


class PowerException(Exception):
   pass


class PowerMan:
   
   def __init__(self, ctrl_socket, mon_socket):
      self.ctrl_socket = ctrl_socket
      self.mon_socket = mon_socket
      self.mon_thread = start_daemon_thread(self.monitor)


   def _exec(self, cmd):
      req = PowerReq()
      req.cmd = cmd
      self.ctrl_socket.send(req.SerializeToString())
      rep.ParseFromString(socket.recv())
      if rep.status != OK:
         if rep.status == E_SYNTAX:
            print 'received reply garbage'
         else:
            raise PowerException


   def stand_power(self):
      self._exec(STAND_POWER)


   def flight_power(self):
      self._exec(FLIGHT_POWER)


   def monitor(self):
      self.state = PowerState()
      while True:
         try:
            data = self.mon_socket.recv()
         except:
            sleep(0.1)
            continue
         try:
            self.state.ParseFromString(data)
         except:
            pass

