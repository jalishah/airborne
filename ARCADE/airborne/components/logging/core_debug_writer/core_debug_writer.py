#!/usr/bin/env python

from os import sep
from scl import generate_map
from misc import daemonize, user_data_dir
from datetime import datetime
import gzip


def main(name):
   socket = generate_map(name)['debug']
   try:
      now = datetime.today().isoformat()
      f = gzip.open(user_data_dir() + sep + 'core_debug_%s.gz' % now, "wb")
      while True:
         line = socket.recv()
         f.write(comp.compress(line + '\n'))
   finally:
      f.close()


daemonize('core_debug_writer', main)

