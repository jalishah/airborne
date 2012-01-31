
import atexit
import os
import readline
import pprint
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
_pp = pprint.PrettyPrinter(indent = 3)


def get(key = ''):
   try:
      _pp.pprint(_interface.get(key))
   except KeyError:
      print('key not found')

def set(key, val):
   try:
      _interface.set(key, val)
   except KeyError:
      print('key not found')

def persist():
   print 'status:', _interface.persist()

