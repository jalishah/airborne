from flight_sm import flight_Standing, flight_Stopping, flight_Taking_off, flight_Landing, flight_Moving, flight_Hovering
from icarus_pb2 import STANDING, TAKING_OFF, STOPPING, LANDING, MOVING, HOVERING


_flight_state_map = {STANDING:   'STANDING',
                     STOPPING:   'STOPPING',
                     TAKING_OFF: 'TAKING_OFF',
                     MOVING:     'MOVING',
                     HOVERING:   'HOVERING',
                     LANDING:    'LANDING'}


def to_string(state):
   return _flight_state_map[state]

