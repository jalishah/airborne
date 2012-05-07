#!/usr/bin/env python

import atexit
import os
import readline
import rlcompleter

from scl import generate_map
from core_pb2 import *
from core_interface import CoreInterface


# set-up command history:
_history = os.path.expanduser("~/.ARCADE_core_cmd_history")
def _save_history(historyPath = _history):
   readline.write_history_file(_history)
if os.path.exists(_history):
   readline.read_history_file(_history)
readline.parse_and_bind("tab: complete")
atexit.register(_save_history)


_ctrl_socket = generate_map('shell')['ctrl']
i = CoreInterface(_ctrl_socket, None)

print 'type help(i) for help'
