
from state_update_pb2 import HOVERING, STANDING
from arbiter_pb2 import TAKEOFF, LAND, ROT, MOVE
from interface import ArbiterInterface
from state import StateReader, StateEmitter


class MissionExecutor:

   def __init__(self, arbiter_socket, state_socket):
      self.interface = ArbiterInterface(arbiter_socket)
      state_reader = StateReader(state_socket)
      self.emitter = StateEmitter(state_reader)
      self.emitter.start()

   def run(self, mission):
      for request in mission:
         type = request.type
         if type == TAKEOFF:
            self.emitter.event[HOVERING].clear()
            self.interface.execute(request)
            self.emitter.event[HOVERING].wait()
         elif type == LAND:
            self.emitter.event[STANDING].clear()
            self.interface.execute(request)
            self.emitter.event[STANDING].wait()
         elif type == MOVE:
            self.emitter.event[HOVERING].clear()
            self.interface.execute(request)
            self.emitter.event[HOVERING].wait()
         elif type == ROT:
            self.interface.execute(request)
         else:
            print 'unknown request type', type

