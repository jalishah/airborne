
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
      self._lock = Lock()

   def execute(self, req):
      rep = Reply()
      req_data = req.SerializeToString()
      self._lock.acquire()
      self._socket.send(req_data)
      rep_data = self._socket.recv()
      self._lock.release()
      rep.ParseFromString(rep_data)
      if rep.status != 0:
         raise ArbiterError('arbiter error: %s - code: %d' % (rep.message, rep.status))

