
# ICARUS client service provider interface
# sends ICARUS commands and reads status


from icarus__pb2 import *


class ICARUS_Error(Exception):

   def __init__(self, msg):
      self.msg = msg

   def __str__(self):
      return self.msg


class ICARUS_Client:

   def __init__(self, socket):
      self._socket = socket

   def _execute(self, req):
      rep = Reply()
      req_data = req.SerializeToString()
      self._socket.send(req_data)
      rep_data = self._socket.recv()
      rep.ParseFromString(rep_data)
      if rep.status != 0:
         raise ICARUS_Error('ICARUS_ error: %s - code: %d' % (rep.message, rep.status))


   def takeoff(self, z, glob, speed):
      """takeoff"""
      req = Request()
      req.type = TAKEOFF
      if alt:
         req.pos.append(alt)
      if speed:
         req.speed = speed
      self._execute(req)


   def land(self, speed):
      """land at current position"""
      req = Request()
      req.type = LAND
      if speed:
         req.speed = speed
      self._execute(req)


   def move(self, pos, glob, rel, speed, block):
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


   def rotate(self, pos, glob, rel, speed, block):
      """rotate UAV towards pos"""
      req = Request()
      req.type = ROT
      req.pos.extend(pos)
      if speed:
         req.speed = speed
      if rel:
         req.rel = rel
      self._execute(req)


   def stop(self):
      """stop UAV at current position"""
      req = Request()
      req.type = STOP
      self._execute(req)


