
# file: requests.py
#
# utility classes for creating arbiter commands
#
# author: Tobias Simon, Ilmenau University of Technology
#


from arbiter_pb2 import Request, TAKEOFF, LAND, MOVE, ROT, STOP


class Takeoff:

   def __init__(self, alt = None, speed = None):
      req = Request()
      req.type = TAKEOFF
      if alt:
         req.pos.append(alt)
      if speed:
         req.speed = speed
      self._req = req


class RotateFixed:

   def __init__(self, yaw, rel = None, speed = None):
      req = Request()
      req.type = ROT
      req.pos.append(yaw)
      if speed:
         req.speed = speed
      if rel:
         req.rel = rel
      self._req = req


class RotatePOI:

   def __init__(self, pos, rel = None, speed = None):
      req = Request()
      req.type = ROT
      req.pos.extend(pos)
      if speed:
         req.speed = speed
      if rel:
         req.rel = rel
      self._req = req


class Land:

   def __init__(self, speed = None):
      req = Request()
      req.type = LAND
      if speed:
         req.speed = speed
      self._req = req


class Move:

   def __init__(self, pos, alt = None, speed = None, rel = None):
      req = Request()
      req.type = MOVE
      if rel:
         req.rel = rel
      req.pos.extend(pos)
      if alt:
         req.pos.append(alt)
      if speed:
         req.speed = speed
      self._req = req


class Stop:

   def __init__(self):
      req = Request()
      req.type = STOP
      self._req = req


import unittest


class TestArbiterRequests(unittest.TestCase):

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

