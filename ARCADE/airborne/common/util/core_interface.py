
from threading import Thread
from core_pb2 import *


class CoreError(Exception):

   def __init__(self, status, err_msg):
      self.status = status
      self.err_msg = err_msg

   def __repr__(self):
      err_map = {E_SYNTAX: 'E_SYNTAX', E_SEMANTIC: 'E_SEMANTIC', E_HARDWARE: 'E_HARDWARE'}
      return 'class: ' + err_map[self.status] + ' message: ' + self.err_msg


class CoreInterface(Thread):

   def __init__(self, ctrl_socket, mon_socket):
      Thread.__init__(self)
      self.ctrl_socket = ctrl_socket
      self.params = self.get_params()
      if mon_socket != None:
         self.mon_socket = mon_socket
         self.mon = MonData()
         self.daemon = True
         self.start()

   def run(self):
      while True:
         data = self.mon_socket.recv()
         self.mon.ParseFromString(data)

   def _exec(self, req):
      self.ctrl_socket.send(req.SerializeToString())
      rep = Reply()
      rep.ParseFromString(self.ctrl_socket.recv())
      if rep.status != 0:
         raise CoreError(rep.status, rep.err_msg)
      return rep

   def spin_up(self):
      req = Request()
      req.type = SPIN_UP
      self._exec(req)

   def spin_down(self):
      req = Request()
      req.type = SPIN_DOWN
      self._exec(req)

   def reset_ctrl(self):
      req = Request()
      req.type = RESET_CTRL
      self._exec(req)
  
   def set_ctrl_param(self, param, val):
      req = Request()
      req.type = SET_CTRL_PARAM
      req.ctrl_data.param = param
      req.ctrl_data.val = val
      self._exec(req)

   def get_params(self):
      req = Request()
      req.type = GET_PARAMS
      rep = self._exec(req)
      return rep.params



