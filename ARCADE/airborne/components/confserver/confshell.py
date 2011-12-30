
#
# file: confserver.py
# purpose: overridable configuration fil based on yaml
#


from config_pb2 import CtrlReq, CtrlRep, Value
from scl import generate_map


def split_identifier(id):
   return id.split('.')


socket = generate_map('confshell')['ctrl']


req = CtrlReq()
req.type = CtrlReq.SET
req.id = 'kalman.process'
req.val.dbl_val = 1.3
socket.send(req.SerializeToString())
rep = CtrlRep()
rep.ParseFromString(socket.recv())
print rep
