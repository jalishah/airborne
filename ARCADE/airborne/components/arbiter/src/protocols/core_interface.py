
from core_pb2 import *
from threading import Thread, Lock


class CoreError(Exception):

   def __init__(self, code, msg):
      self.code = code
      self.msg = msg


class CoreInterface:

   def __init__(self, socket):
      self._socket = socket
      self._lock = Lock()

   def _exec(self, req):
      self._lock.acquire()
      self._socket.send(req.SerializeToString())
      rep = Reply()
      rep.ParseFromString(self._socket.recv())
      self._lock.release()
      
      if rep.status != 0:
         raise CoreError(rep.status, rep.message)
      return rep

   def power_on(self):
      req = Request()
      req.type = POWER_ON
      self._exec(req).status

   def power_off(self):
      req = Request()
      req.type = POWER_OFF
      self._exec(req).status

   def start_motors(self):
      req = Request()
      req.type = START_MOTORS
      self._exec(req).status

   def stop_motors(self):
      req = Request()
      req.type = STOP_MOTORS
      self._exec(req).status
   
   def set_altitude(self, alt, alt_type, speed = None):
      req = Request()
      req.type = CTRL_SET_SP
      req.ctrl.type = ALT
      req.ctrl.alt_type = alt_type
      req.ctrl.pos.append(alt)
      if speed:
         req.ctrl.speed = speed
      self._exec(req)
   
   def set_yaw(self, yaw, speed = None):
      req = Request()
      req.type = CTRL_SET_SP
      req.ctrl.type = YAW
      req.ctrl.pos.append(yaw)
      if speed:
         req.ctrl.speed = speed
      self._exec(req)

   def set_gps(self, pos, speed = None):
      req = Request()
      req.type = CTRL_SET_SP
      req.ctrl.type = GPS
      for item in pos:
         req.ctrl.pos.append(item)
      if speed:
         req.ctrl.speed = speed
      self._exec(req)

   def reset_controllers(self):
      req = Request()
      req.type = CTRL_RESET
      self._exec(req)

   def _exec_and_prepare_output(self, req):
      val = self._exec(req).value
      if len(val) == 1:
         return val[0]
      else:
         assert len(val) == 2
         return val[0], val[1]

   def get_state(self, type):
      req = Request()
      req.type = STATE_GET
      req.state_type = type
      return self._exec_and_prepare_output(req)

   def get_ctrl_error(self, type):
      req = Request()
      req.type = CTRL_GET_ERR
      req.ctrl.type = type
      return self._exec_and_prepare_output(req)


import unittest


class TestRequests(unittest.TestCase):

   def test_takeoff(self):
      Takeoff(alt = None, speed = None)
      Takeoff(alt = 1,    speed = None)
      Takeoff(alt = None, speed = 1)
      Takeoff(alt = 1,    speed = 1)
 
   def test_rotate_fixed(self):
      RotateFixed(1, rel = True, speed = None)
      RotateFixed(1, rel = None, speed = None)
      RotateFixed(1, rel = True, speed = 1)
      RotateFixed(1, rel = None, speed = 1)
 
   def test_rotate_poi(self):
      RotatePOI((0, 0), rel = True, speed = None)
      RotatePOI((0, 0), rel = None, speed = None)
      RotatePOI((0, 0), rel = True, speed = 1)
      RotatePOI((0, 0), rel = None, speed = 1)
   
   def test_land(self):
      Land()
      Land(1)

   def test_move(self):
      Move((0, 0), alt = None, speed = None, rel = None)
      Move((0, 0), alt = 1,    speed = None, rel = None)
      Move((0, 0), alt = None, speed = 1,    rel = None)
      Move((0, 0), alt = 1,    speed = 1,    rel = None)
      Move((0, 0), alt = None, speed = None, rel = True)
      Move((0, 0), alt = 1,    speed = None, rel = True)
      Move((0, 0), alt = None, speed = 1,    rel = True)
      Move((0, 0), alt = 1,    speed = 1,    rel = True)


if __name__ == '__main__':
   unittest.main()


