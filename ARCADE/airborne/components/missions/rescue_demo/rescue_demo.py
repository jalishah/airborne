#!/usr/bin/env python


from zmq import Context, REP, PUB
from mission.manager import MissionManager
from mission.localization import LocMission
from mission.placement import PlaceMission
from scl import generate_map
from mission_pb2 import MissionMessage


def main(name):
   map = generate_map(name)
   context = Context()
   pub_socket = context.socket(PUB)
   pub_socket.bind('tcp://0.0.0.0:20000')
   map['pub_server'] = pub_socket
   rep_socket = context.socket(REP)
   rep_socket.bind('tcp://0.0.0.0:20001')
   map['rep_server'] = rep_socket
   manager = MissionManager()
   rep_socket = map['rep_server']
   manager.start_mission(LocMission(map, None))
   return
   while True:
      req = MissionMessage()
      req.ParseFromString(rep_socket.recv())
      if req.type == 6:
         req.type = 7
         try:
            if req.missionType == MissionMessage.CONNECTION:
               manager.start_mission(PlaceMission(map, (0.0, 0.0), req))
            elif req.missionType == MissionMessage.LOCALIZATION:
               manager.start_mission(LocMission(map, req))
            else:
               raise ValueError('unknown mission type')
            req.status = MissionMessage.ACTIVE
         except RuntimeError:
            req.status = MissionMessage.REJECTED
         rep_socket.send(req.SerializeToString())


main('demo_mission')

