#!/usr/bin/env python


from named_daemon import daemonize
from scl import generate_map
from protocols.icarus_server import ICARUS_Server
from core_interface import CoreInterface
from protocols.state_emitter import StateEmitter
from event_handler import EventHandler

from power_pb2 import PowerState, PowerReq, PowerRep, FLYING, STANDING
from threading import Thread

class PowerReader(Thread):
   
   def __init__(self, socket):
      Thread.__init__(self)
      self.socket = socket

   def run(self):
      while True:
         state = PowerState()
         data = self.socket.recv()
         state.ParseFromString(data)
         print state


def main(name):
   sockets = generate_map(name)
   r = PowerReader(sockets['power_mon'])
   r.start()
   req = PowerReq()
   req.command = STANDING
   #req.command = FLYING
   socket = sockets['power_ctrl']
   socket.send(req.SerializeToString())
   rep = PowerRep()
   rep.ParseFromString(socket.recv())
   #core = CoreInterface(sockets['core'], sockets['mon'])
   #state_emitter = StateEmitter(sockets['hlsm'])
   #handler = EventHandler(core, state_emitter)
   #icarus_srv = ICARUS_Server(sockets['ctrl'], handler)
   #icarus_srv.run()


main('icarus')
#daemonize('arbiter', main)

