

from core_pb2 import *


class CoreError(Exception):

   def __init__(self, status):
      self.status = status


class CoreInterface:

   def __init__(self, socket):
      self._socket = socket

   def _exec(self, req):
      self._socket.send(req.SerializeToString())
      rep = Reply()
      rep.ParseFromString(self._socket.recv())
      if rep.status != 0:
         raise CoreError(rep.status)
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

   def get_value(self, val_id):
      req = Request()
      req.type = GET_VAL
      req.val_id = val_id
      return self._exec(req).val


from scl import generate_map

if __name__ == '__main__':
   sockets = generate_map('arbiter')
   core = CoreInterface(sockets['core'])
   print core.get_state(GPS_START_LON)

