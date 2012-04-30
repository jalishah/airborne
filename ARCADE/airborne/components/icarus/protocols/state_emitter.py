
from flight_sm import flight_Standing, flight_Taking_off, flight_Landing, flight_Moving, flight_Hovering
from state_update_pb2 import StateUpdate, STANDING, TAKING_OFF, LANDING, MOVING, HOVERING


class StateEmitter:

   def __init__(self, socket):
      self._socket = socket
      self._map = {  flight_Standing:   STANDING,
                     flight_Taking_off: TAKING_OFF,
                     flight_Moving:     MOVING,
                     flight_Hovering:   HOVERING,
                     flight_Landing:    LANDING     }


   def send(self, state):
      sm = StateUpdate()
      sm.state = self._map[state.__class__]
      print 'new state:', state.getName()
      self._socket.send(sm.SerializeToString())

