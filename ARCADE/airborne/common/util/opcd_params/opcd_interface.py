
#
# file: opcd_interface.py
# purpose: OPCD interface class
#


from opcd_pb2 import CtrlReq, CtrlRep, Value
from scl import generate_map
from threading import Lock


class OPCD_Interface:


   def __init__(self, socket, prefix = None):
      self.socket = socket
      self.prefix = prefix
      self.map = {str: 'str_val', int: 'int_val', float: 'dbl_val', bool: 'bool_val'}
      self.lock = Lock()

   def _send_and_recv(self, req):
      self.lock.acquire()
      self.socket.send(req.SerializeToString())
      rep = CtrlRep()
      rep.ParseFromString(self.socket.recv())
      self.lock.release()
      return rep


   def get(self, id):
      req = CtrlReq()
      req.type = CtrlReq.GET
      if self.prefix:
         id = self.prefix + '.' + id
      req.id = id
      rep = self._send_and_recv(req)
      if rep.status != 0:
         raise KeyError
      pairs = []
      for pair in rep.pairs:
         for type in self.map.values():
            if pair.val.HasField(type):
               val = getattr(pair.val, type)
               if type == 'str_val':
                  val = val.encode('ascii')
               pairs.append((pair.id.encode('ascii'), val))
               break
      if len(pairs) == 0:
         return
      elif len(pairs) == 1:
         return pairs[0][1]
      else:
         return pairs


   def set(self, id, val):
      req = CtrlReq()
      req.type = CtrlReq.SET
      if self.prefix:
         id = self.prefix + '.' + id
      req.id = id
      setattr(req.val, self.map[val.__class__], val)
      rep = self._send_and_recv(req)
      if rep.status != 0:
         raise KeyError


   def persist(self):
      req = CtrlReq()
      req.type = CtrlReq.PERSIST
      return self._send_and_recv(req).status

