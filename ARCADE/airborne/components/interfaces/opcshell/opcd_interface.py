
#
# file: opcd_interface.py
# purpose: OPCD interface class
#


from opcd_pb2 import CtrlReq, CtrlRep, Value
from scl import generate_map


class OPCD_Interface:


   def __init__(self, name):
      self.socket = generate_map(name)['ctrl']
      self.map = {str: 'str_val', int: 'int_val', float: 'dbl_val', bool: 'bool_val'}

   def _send_and_recv(self, req):
      self.socket.send(req.SerializeToString())
      rep = CtrlRep()
      rep.ParseFromString(self.socket.recv())
      return rep


   def get(self, id):
      req = CtrlReq()
      req.type = CtrlReq.GET
      req.id = id
      rep = self._send_and_recv(req)
      assert rep.status == 0
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
      req.id = id
      setattr(req.val, self.map[val.__class__], val)
      return self._send_and_recv(req).status


   def persist(self):
      req = CtrlReq()
      req.type = CtrlReq.PERSIST
      return self._send_and_recv(req).status

