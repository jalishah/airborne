#!/usr/bin/env python

import atexit
import os
import readline
import rlcompleter

from scl import generate_map
from icarus_client import ICARUS_Client
from icarus_interface import ICARUS_Interface


# set-up command history:
_history = os.path.expanduser("~/.ARCADE_cmdshell_history")
def _save_history(historyPath = _history):
   readline.write_history_file(_history)
if os.path.exists(_history):
   readline.read_history_file(_history)
readline.parse_and_bind("tab: complete")
atexit.register(_save_history)


# define
_socket = generate_map('cmdshell')['ctrl']
_client = ICARUS_Client(_socket)
i = ICARUS_Interface(_client)

