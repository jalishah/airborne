
from icarus_pb2 import *
from icarus_interface import ICARUS_Client, ICARUS_MissionFactory
from state_reader import StateReader, StateEventMap


class MissionExecutor:

   def __init__(self, ctrl_socket, state_socket):
      self.interface = ICARUS_Client(ctrl_socket)
      reader = StateReader(state_socket)
      self.map = StateEventMap(reader)
      self.map.start()

   def run(self, mission):
      for request in mission:
         type = request.type
         if type == TAKEOFF:
            print 'TAKEOFF'
            self.map.events[HOVERING].clear()
            self.interface.execute(request)
            self.map.events[HOVERING].wait()
         elif type == LAND:
            print 'LAND'
            self.map.events[STANDING].clear()
            self.interface.execute(request)
            self.map.events[STANDING].wait()
         elif type == MOVE:
            print 'MOVE'
            self.map.events[HOVERING].clear()
            self.interface.execute(request)
            self.map.events[HOVERING].wait()
         elif type == ROT:
            print 'ROT'
            self.interface.execute(request)
         else:
            print 'unknown request type', type

from scl import generate_map

map = generate_map('ferry_ctrl')
executor = MissionExecutor(map['ctrl'], map['state'])
f = ICARUS_MissionFactory()


def mission():
   for x in [0, 1]:
      for y in [0, 1]:
         yield f.move_xy(x, y)

executor.run(mission())

