#!/usr/bin/env python


from named_daemon import daemonize
from scl import generate_map
from protocols.icarus_server import ICARUS_Server
from core_interface import CoreInterface
from protocols.state_emitter import StateEmitter
from event_handler import EventHandler
from protocols.powerman import PowerMan
from mtputil import await_signal


def main(name):
   sockets = generate_map(name)
   core = CoreInterface(sockets['core'], sockets['mon'])
   state_emitter = StateEmitter(sockets['hlsm'])
   powerman = PowerMan(sockets['power_ctrl'], sockets['power_mon'])
   handler = EventHandler(core, state_emitter, powerman)
   icarus_srv = ICARUS_Server(sockets['ctrl'], handler)
   icarus_srv.start()
   await_signal()


#main('icarus')
daemonize('icarus', main)

