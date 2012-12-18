#!/usr/bin/env python

from os import sep, symlink, unlink
from scl import generate_map
from misc import daemonize, user_data_dir
from datetime import datetime


def main(name):
   socket = generate_map(name)['debug']
   try:
      now = datetime.today().isoformat().replace(':', '')
      symlink_file = user_data_dir() + sep + 'core_debug.msgpack'
      try:
         unlink(symlink_file)
      except:
         pass
      new_file = user_data_dir() + sep + 'core_debug_%s.msgpack' % now
      symlink(new_file, symlink_file)
      f = open(new_file, "wb")
      while True:
         f.write(socket.recv())
   finally:
      try:
         f.close()
      except:
         pass

daemonize('core_debug_writer', main)

