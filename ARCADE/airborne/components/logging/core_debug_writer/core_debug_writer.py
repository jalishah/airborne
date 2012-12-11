#!/usr/bin/env python

from os import sep
from scl import generate_map
from misc import daemonize, user_data_dir
from datetime import datetime


def main(name):
   socket = generate_map(name)['debug']
   try:
      now = datetime.today().isoformat().replace(':', '')
      f = open(user_data_dir() + sep + 'core_debug_%s.log' % now, "wb")
      while True:
         f.write(socket.recv())
   finally:
      f.close()


daemonize('core_debug_writer', main)

