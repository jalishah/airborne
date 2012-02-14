
# arbiter interface library
# sends arbiter commands and reads status


from arbiter_pb2 import *
from threading import Lock 


class ArbiterError(Exception):

   def __init__(self, msg):
      self.msg = msg

   def __str__(self):
      return self.msg


class ArbiterInterface:

   def __init__(self, socket):
      self._socket = socket

   def _execute(self, req):
      rep = Reply()
      req_data = req.SerializeToString()
      self._socket.send(req_data)
      rep_data = self._socket.recv()
      rep.ParseFromString(rep_data)
      if rep.status != 0:
         raise ArbiterError('arbiter error: %s - code: %d' % (rep.message, rep.status))

   def takeoff(alt = None, speed = None):
      """takeoff"""
      req = Request()
      req.type = TAKEOFF
      if alt:
         req.pos.append(alt)
      if speed:
         req.speed = speed
      self._execute(req)


   def land(speed = None):
      """land at current position"""
      req = Request()
      req.type = LAND
      if speed:
         req.speed = speed
      self._execute(req)


   def move(pos, alt = None, speed = None, rel = None):
      """move to position"""
      req = Request()
      req.type = MOVE
      if rel:
         req.rel = rel
      req.pos.extend(pos)
      if alt:
         req.pos.append(alt)
      if speed:
         req.speed = speed
      self._execute(req)


   def rotate_poi(pos, rel = None, speed = None):
      """rotate UAV towards pos"""
      req = Request()
      req.type = ROT
      req.pos.extend(pos)
      if speed:
         req.speed = speed
      if rel:
         req.rel = rel
      self._execute(req)


   def rotate_fixed(yaw, rel = None, speed = None):
      """rotate UAV towards fixed angle"""
      req = Request()
      req.type = ROT
      req.pos.append(yaw)
      if speed:
         req.speed = speed
      if rel:
         req.rel = rel
      self._execute(req)


   def stop():
      """stop UAV at current position"""
      req = Request()
      req.type = STOP
      self._execute(req)


