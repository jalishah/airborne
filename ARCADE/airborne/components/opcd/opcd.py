#!/usr/bin/env python


#
# file: opcd.py
# purpose: (O)nline (P)arameter (C)onfiguration (D)aemon
#          reads and applies changes to a YAML configuration overlay file
# author: Tobias Simon, Ilmenau University of Technology
#


from config import Config, ConfigError
from opcd_pb2 import CtrlReq, CtrlRep, Pair
from scl import generate_map
from named_daemon import daemonize
from sys import argv
from re import match


class OPCD:


   def __init__(self, name):
      map = generate_map(name)
      self.ctrl_socket = map['ctrl']
      self.event_socket = map['event']
      self.conf = Config()
      self.map = {str: 'str_val', int: 'int_val', float: 'dbl_val', bool: 'bool_val'}


   def _pairs_add(self, key, rep):
      try:
         val = self.conf.get(key)
         pair = rep.pairs.add()
         pair.id = key
         setattr(pair.val, self.map[val.__class__], val)
      except ConfigError:
         rep.status = CtrlRep.PARAM_UNKNOWN
      except ValueError: # TODO: check: does this still apply?
         rep.status = CtrlRep.MALFORMED_ID


   def run(self):

      while True:
         # read and parse request:
         req = CtrlReq()
         req.ParseFromString(self.ctrl_socket.recv())

         # process request and prepare reply:
         rep = CtrlRep()
         rep.status = CtrlRep.OK

         # GET REQUEST:
         if req.type == CtrlReq.GET:
            all_keys = self.conf.get_all_keys(self.conf.base)
            if req.id in all_keys:
               # exact match:
               self._pairs_add(req.id, rep)
            else:
               # try regex matches:
               found = False
               for key in all_keys:
                  try:
                     if match(req.id, key):
                        self._pairs_add(key, rep)
                        found = True
                  except:
                     pass
               if not found:
                  rep.status = CtrlRep.PARAM_UNKNOWN

         # SET REQUEST:
         elif req.type == CtrlReq.SET:
            try:
               for type, attr in self.map.items():
                  if req.val.HasField(attr):
                     val = getattr(req.val, attr)
                     self.conf.set(req.id.encode('ascii'), type(val))
                     break
               pair = Pair(id = req.id, val = req.val)
               self.event_socket.send(pair.SerializeToString())
            except ConfigError, e:
               rep.status = CtrlRep.PARAM_UNKNOWN

         # PERSIST REQUEST:
         else:
            assert req.type == CtrlReq.PERSIST
            try:
               self.conf.persist()
               rep.status = CtrlRep.OK
            except:
               rep.status = CtrlRep.IO_ERROR

         # send reply:
         self.ctrl_socket.send(rep.SerializeToString())


def main(name):
   opcd = OPCD(name)
   opcd.run()


daemonize('opcd', main)

