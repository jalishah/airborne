#!/usr/bin/env python

from named_daemon import daemonize
from scl import generate_map
from dispatcher import Dispatcher
from protocols.arbiter_driver import ArbiterProtocolDriver
from protocols.core_interface import CoreInterface
from protocols.state_update_interface import StateUpdateInterface

def main(name):
   sockets = generate_map(name)
   core = CoreInterface(sockets['core'])
   sui = StateUpdateInterface(sockets['hlsm'])
   disp = Dispatcher(core, sui)
   apd = ArbiterProtocolDriver(sockets['ctrl'], disp)
   apd.run()

daemonize('arbiter', main)

