#!/usr/bin/env python


from named_daemon import daemonize
from scl import generate_map
from protocols.icarus_server import ICARUS_Server
from protocols.core_interface import CoreInterface
from protocols.state_emitter import StateEmitter
from event_handler import EventHandler


def main(name):
   sockets = generate_map(name)
   core = CoreInterface(sockets['core'], sockets['mon'])
   state_emitter = StateEmitter(sockets['hlsm'])
   handler = EventHandler(core, state_emitter)
   icarus_srv = ICARUS_Server(sockets['ctrl'], handler)
   icarus_srv.run()


main('icarus')
#daemonize('arbiter', main)

