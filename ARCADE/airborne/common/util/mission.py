
from icarus_pb2 import *
from icarus_pb2 import _STATE
from icarus_interface import ICARUS_Client, ICARUS_MissionFactory
from threading import Thread, Event



class StateEventMap(Thread):

   """
   reads state updates and
   publishes them using the "events" dictionary.
   clients can use emitter.event[name].clear/wait in order
   to wait for an event
   """

   def __init__(self, socket):
      Thread.__init__(self)
      self._socket = socket
      self.daemon = True
      self.events = {}
      for state in _STATE.values:
         self.events[state.number] = Event()
      self.name_map = {STANDING:   'STANDING',
                       STOPPING:   'STOPPING',
                       TAKING_OFF: 'TAKING_OFF',
                       MOVING:     'MOVING',
                       HOVERING:   'HOVERING',
                       LANDING:    'LANDING'}


   def run(self):
      while True:
         raw_data = self._socket.recv()
         su = StateUpdate()
         su.ParseFromString(raw_data)
         print 'new state:', self.name_map[su.state]
         self.events[su.state].set()



class MissionExecutor:

   def __init__(self, ctrl_socket, state_socket):
      self.interface = ICARUS_Client(ctrl_socket)
      self.map = StateEventMap(state_socket)
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
   yield f.land()

executor.run(mission())

