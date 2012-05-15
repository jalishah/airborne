#!/usr/bin/env python

import atexit
import os
import readline
import rlcompleter

from scl import generate_map
from core_pb2 import *
from core_interface import CoreInterface
from misc import user_data_dir


# set-up command history:
_path = user_data_dir() + os.sep + 'core_shell.history'
_history = os.path.expanduser(_path)
def _save_history(historyPath = _history):
   readline.write_history_file(_history)
if os.path.exists(_history):
   readline.read_history_file(_history)
readline.parse_and_bind("tab: complete")
atexit.register(_save_history)

def monitor():
   mon_data = MonData()
   try:
      while True:
         i.mon_read(mon_data)
         print mon_data
   except:
      pass

_map = generate_map('core_shell')
_ctrl_socket = _map['ctrl']
_mon_socket = _map['mon']
i = CoreInterface(_ctrl_socket, _mon_socket)


print 'type help(i) for help'
