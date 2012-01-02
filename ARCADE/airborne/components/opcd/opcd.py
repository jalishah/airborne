#!/usr/bin/env python


#
# file: opcd.py
# purpose: (O)nline (P)arameter (C)onfiguration (D)aemon
#          reads and applies changes to a YAML configuration overlay file
# author: Tobias Simon, Ilmenau University of Technology
#


from config import Config, ConfigError
from config_pb2 import CtrlReq, CtrlRep, Value, Pair
from scl import generate_map
#from named_daemon import daemonize
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
         matches = []
         for key in conf.get_all_keys(conf.base):
            if key.find(req.id) == 0:
               try:
                  val = conf.get(key)
                  pair = rep.pairs.add()
                  pair.id = key
                  if isinstance(val, str):
                     pair.val.str_val = val
                  elif isinstance(val, int):
                     pair.val.int_val = val
                  elif isinstance(val, float):
                     pair.val.dbl_val = val
                  else:
                     assert isinstance(val, bool)
                     pair.val.bool_val = val
               except ConfigError:
                  rep.status = CtrlRep.PARAM_UNKNOWN
               except ValueError:
                  rep.status = CtrlRep.MALFORMED_ID
                  print conf.get(match)
      
      elif req.type == CtrlReq.SET:
         map = {Value.STR: ('str_val', str),
                Value.INT: ('int_val', int),
                Value.DBL: ('dbl_val', float),
                Value.BOOL: ('bool_val', bool)}
         entry = map[req.val.type]
         val = getattr(req.val, entry[0])
         conf.set(req.id.encode('ascii'), entry[1](val))
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
