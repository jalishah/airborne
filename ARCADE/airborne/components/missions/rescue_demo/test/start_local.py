
from zmq import Context, REQ, SUB, SUBSCRIBE
from mission_pb2 import MissionMessage


context = Context()
socket = context.socket(REQ)
socket.connect('tcp://0.0.0.0:20001')
req = MissionMessage()
req.type = 6
req.missionType = MissionMessage.LOCALIZATION
socket.send(req.SerializeToString())
req.ParseFromString(socket.recv())
print req

socket = context.socket(SUB)
socket.setsockopt(SUBSCRIBE, '')
socket.connect('tcp://0.0.0.0:20002')
req = MissionMessage()
while True:
   req.ParseFromString(socket.recv())
   print req

