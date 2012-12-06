#!/usr/bin/env python

from os import sep
from scl import generate_map
from misc import daemonize, user_data_dir
from lzma import LZMACompressor
from datetime import datetime


def main(name):
   socket = generate_map(name)['debug']
   comp = LZMACompressor()
   try:
      now = datetime.today().isoformat()
      f = open(user_data_dir() + sep + 'core_debug_%s.xz' % now, "wb")
      while True:
         line = socket.recv()
         f.write(comp.compress(line + '\n'))
         f.flush()
   finally:
      f.write(comp.flush())
      f.close()


daemonize('core_debug_writer', main)

