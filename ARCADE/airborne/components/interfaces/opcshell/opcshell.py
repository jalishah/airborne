
import atexit
import os
import readline

from arbiter_requests import Takeoff, Land, Move, RotateFixed, RotatePOI, Stop
from arbiter_interface import ArbiterInterface
from opcd_interface import OPCD_Interface


# set-up command history:
_history = os.path.expanduser("~/.ARCADE_opcd_shell_history")
def _save_history(historyPath = _history):
   readline.write_history_file(_history)
if os.path.exists(_history):
   readline.read_history_file(_history)
readline.parse_and_bind("tab: complete")
atexit.register(_save_history)


#initialize and define interface:
_interface = OPCD_Interface('opcd_shell')

def get(key):
   return _interface.get(key)

def set(key, val):
   return _interface.set(key, val)

def persist():
   return _interface.persist()

