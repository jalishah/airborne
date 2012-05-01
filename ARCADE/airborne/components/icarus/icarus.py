#!/usr/bin/env python


from named_daemon import daemonize
from scl import generate_map
from protocols.icarus_server import ICARUS_Server
from protocols.core_mon import CoreMon
from protocols.core_ctrl import CoreInterface
from protocols.state_emitter import StateEmitter
from flight_manager import FlightManager


def main(name):
   sockets = generate_map(name)
   core = CoreInterface(sockets['core'])
   monitor = CoreMon(sockets['mon'])
   state_emitter = StateEmitter(sockets['hlsm'])
   manager = FlightManager(core, state_emitter, monitor)
   icarus_srv = ICARUS_Server(sockets['ctrl'], manager)
   icarus_srv.run()


main('icarus')
#daemonize('arbiter', main)

