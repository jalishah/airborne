
from flight_sm import flight_Standing, flight_Taking_off, flight_Landing, flight_Moving, flight_Hovering
from icarus_pb2 import STANDING, TAKING_OFF, LANDING, MOVING, HOVERING


_flight_state_map = {flight_Standing:   'STANDING',
                     flight_Taking_off: 'TAKING_OFF',
                     flight_Moving:     'MOVING',
                     flight_Hovering:   'HOVERING',
                     flight_Landing:    'LANDING'}


def to_string(state):
   return _flight_state_map[state.__class__]


def to_protocol(state):
   return globals()[to_string(state)]


