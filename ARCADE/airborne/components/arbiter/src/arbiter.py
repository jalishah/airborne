#!/usr/bin/env python

from named_daemon import daemonize
from scl import generate_map
from flight_manager import FlightManager, StateMachineError
from protocols.arbiter_driver import ArbiterProtocolDriver
from protocols.core_interface import CoreInterface
from protocols.state_update_interface import StateUpdateInterface


def main(name):
   sockets = generate_map(name)
   core = CoreInterface(sockets['core'])
   sui = StateUpdateInterface(sockets['hlsm'])
   mgr = FlightManager(core, sui)
   apd = ArbiterProtocolDriver(sockets['ctrl'], mgr)
   while True:
      try:
         apd.handle()
      except ValueError, e: # may be raised by handler methods on invalid parameter values
         apd.send_err(-1, str(e))
      except StateMachineError: # raised by the state machine in case of an invalid transition
         apd.send_err(-2, 'command rejected by state machine')
      except CoreError, e:
         apd.send_err(-3, 'core error')

main('arbiter')
daemonize('arbiter', main)

