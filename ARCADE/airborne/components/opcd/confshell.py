
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
      if rep.val.str_val != None:
         val = rep.val.str_val
      elif rep.val.int_val != None:
         val = rep.val.int_val
      elif rep.val.dbl_val != None:
         val = rep.val.dbl_val
      else:
         assert rep.val.bool_val != None
         val = rep.val.bool_val
      return val


   def set(self, id, val):
      req = CtrlReq()
      req.type = CtrlReq.SET
      req.id = id
      map = {str: (Value.STR, 'str_val'),
             int: (Value.INT, 'int_val'),
             float: (Value.DBL, 'dbl_val'),
             bool: (Value.BOOL, 'bool_val')}
      req.val.type =  map[val.__class__][0]
      setattr(req.val, map[val.__class__][1], val)
      print req
      return self._send_and_recv(req).status


   def persist(self):
      req = CtrlReq()
      req.type = CtrlReq.PERSIST
      return self._send_and_recv(req).status


if __name__ == '__main__':
   opcdi = OPCD_Interface('opcd_shell')
   opcdi.set('core.controllers.yaw.manual', True)
   opcdi.set('core.controllers.yaw.speed_p', 1.0)
   opcdi.set('core.logger.level', 0)
   opcdi.persist()

