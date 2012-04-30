#!/usr/bin/env python


from named_daemon import daemonize
from scl import generate_map
from flight_manager import FlightManager, StateMachineError
from protocols.icarus_server import ICARUS_Server
from protocols.core_mon import CoreMon
from protocols.core_ctrl import CoreInterface, CoreError
from protocols.state_emitter import StateEmitter


def main(name):
   sockets = generate_map(name)
   core = CoreInterface(sockets['core'])
   mon = CoreMon(sockets['mon'])
   mon.start()
   mon.join()
   sui = StateEmitter(sockets['hlsm'])
   mgr = FlightManager(core, mon, sui)
   ipd = ICARUS_Server(sockets['ctrl'], mgr)

   while True:
      try:
         ipd.handle()
      except ValueError, e: # may be raised by handler methods on invalid parameter values
         ipd.send_err(-1, str(e))
      except StateMachineError: # raised by the state machine in case of an invalid transition
         ipd.send_err(-2, 'command rejected by state machine')
      except CoreError, e:
         ipd.send_err(-3, 'core error')

main('icarus')
#daemonize('arbiter', main)

