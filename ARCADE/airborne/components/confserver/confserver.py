
#
# file: confserver.py
# purpose: interface to configuration file based on yaml
#


from config import Config, ConfigError
from config_pb2 import CtrlReq, CtrlRep, Value
from scl import generate_map


def split_identifier(id):
   return id.split('.')


# load config base/overlay:
conf = Config('config')

# retrieve sockets
map = generate_map('confserver')
ctrl_socket = map['ctrl']
update_socket = map['update']

# handle requests:
while True:
   
   # read and parse request:
   req = CtrlReq()
   req.ParseFromString(ctrl_socket.recv())
   
   # process request and prepare reply:
   rep = CtrlRep()
   rep.status = CtrlRep.OK
   
   if req.type == CtrlReq.GET:
      try:
         category, key = split_identifier(req.id)
         val = conf.get(category, key)
         if isinstance(val, str):
            rep.val.str_val = val
         elif isinstance(val, int):
            rep.val.int_val = val
         else:
            assert isinstance(val, float)
            rep.val.dbl_val = val
      except ConfigError:
         rep.status = CtrlRep.PARAM_UNKNOWN
      except ValueError:
         rep.status = CtrlRep.MALFORMED_ID
   
   elif req.type == CtrlReq.SET:
      category, key = split_identifier(req.id)
      if req.val.str_val:
         val = req.val.str_val
      elif req.val.int_val:
         val = req.val.int_val
      else:
         assert req.val.dbl_val
         val = req.val.dbl_val
      conf.set(category, key, req.val)
      update_socket.send()

   else:
      assert req.type == CtrlReq.PERSIST
      try:
         conf.persist()
         rep.status = rep.status = CtrlRep.OK
      except:
         rep.status = CtrlRep.IO_ERROR
   # send reply:
   ctrl_socket.send(rep.SerializeToString())

