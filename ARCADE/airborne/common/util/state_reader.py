
# file: state_reader.py
#
# purpose: reads state updates and returns them in a generator fashion
# author: Tobias Simon, Ilmenau University of Technology


from threading import Thread, Event
from state_update_pb2 import _STATE, StateUpdate


class StateReader:

   def __init__(self, socket):
      self._socket = socket

   def generator(self):
      while True:
         raw_data = self._socket.recv()
         su = StateUpdate()
         su.ParseFromString(raw_data)
         yield su.state


class StateEventMap(Thread):

   """uses a StateReader for reading state updates and
      publishes them using the event dictionary.
      clients can use emitter.event[name].clear/wait in order
      to wait for an event"""

   def __init__(self, state_reader):
      Thread.__init__(self)
      self.daemon = True
      self.reader = state_reader
      self.event = {}
      for state in _STATE.values:
         self._map[state.number] = Event()

   def run(self):
      for state in self.reader.generate():
         self.state = state
         self.event[state].set()

