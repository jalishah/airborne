#!/usr/bin/env python


#
# file: opcd.py
# purpose: (O)nline (P)arameter (C)onfiguration (D)aemon
#          reads and applies changes to a YAML configuration overlay file
# author: Tobias Simon, Ilmenau University of Technology
#


from config import Config, ConfigError
from config_pb2 import CtrlReq, CtrlRep, Value
from scl import generate_map
from named_daemon import daemonize
from sys import argv


def main(name):
   # retrieve sockets:
   map = generate_map(name)
   ctrl_socket = map['ctrl']
   update_socket = map['event']

   # load config base/overlay:
   conf = Config(argv[1])

   # handle requests:
   while True:
      # read and parse request:
      req = CtrlReq()
      req.ParseFromString(ctrl_socket.recv())
      
      # process request and prepare reply:
      rep = CtrlRep()
      rep.status = CtrlRep.OK
      if req.type == CtrlReq.GET:
         try:
            val = conf.get(req.id)
            if isinstance(val, str):
               rep.val.str_val = val
            elif isinstance(val, int):
               rep.val.int_val = val
            else:
               assert isinstance(val, float)
               rep.val.dbl_val = val
         except ConfigError:
            rep.status = CtrlRep.PARAM_UNKNOWN
         except ValueError:
            rep.status = CtrlRep.MALFORMED_ID
      
      elif req.type == CtrlReq.SET:
         if hasattr(req.val, 'str_val'):
            val = req.val.str_val
         elif hasattr(req.val, 'int_val'):
            val = req.val.int_val
         elif hasattr(req.val, 'dbl_val'):
            val = req.val.dbl_val
         else:
            assert hasattr(req.val.bool_val)
            val = req.val.bool_val
         conf.set(req.id, val)
         #update_socket.send()

      else:
         assert req.type == CtrlReq.PERSIST
         try:
            conf.persist()
            rep.status = CtrlRep.OK
         except:
            rep.status = CtrlRep.IO_ERROR
 
      # send reply:
      ctrl_socket.send(rep.SerializeToString())


if len(argv) != 2:
   print 'expected configuration file prefix as first argument'
#daemonize('opcd', main)
main('opcd')
