
#
# file: confserver.py
# purpose: overridable configuration fil based on yaml
#


from config_pb2 import CtrlReq, CtrlRep, Value
from scl import generate_map


class OPCD_Interface:


   def __init__(self, name):
      self.socket = generate_map(name)['ctrl']

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
      if rep.val.str_val:
         val = rep.val.str_val
      elif rep.val.int_val:
         val = rep.val.int_val
      elif rep.val.dbl_val:
         val = rep.val.dbl_val
      else:
         val = rep.val.bool_val
         assert val
      return val


   def set(self, id, val):
      req = CtrlReq()
      req.type = CtrlReq.SET
      req.id = id
      map = {str: 'str_val', int: 'int_val', float: 'dbl_val', bool: 'bool_val'}
      setattr(req.val, map[val.__class__], val)
      print req
      return self._send_and_recv(req).status


   def persist(self):
      req = CtrlReq()
      req.type = CtrlReq.PERSIST
      return self._send_and_recv(req).status


if __name__ == '__main__':
   opcdi = OPCD_Interface('opcd_shell')
   print opcdi.set('core.controllers.angle.p', 13.0)
   print opcdi.set('core.logger.level', 0)
   print opcdi.persist()

