
# ICARUS client service provider interface
# sends ICARUS commands and reads status


from icarus_pb2 import *


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
      req = Request()
      req.type = TAKEOFF
      if not z is None:
         req.takeoff_data.z = z
      if not glob is None:
         req.glob = glob
      if not speed is None:
         req.speed = speed
      self._execute(req)


   def land(self, speed):
      req = Request()
      req.type = LAND
      if speed:
         req.speed = speed
      self._execute(req)


   def move(self, pos, glob, rel, speed, block):
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
      req = Request()
      req.type = ROT
      req.pos.extend(pos)
      if speed:
         req.speed = speed
      if rel:
         req.rel = rel
      self._execute(req)


   def stop(self):
      req = Request()
      req.type = STOP
      self._execute(req)

