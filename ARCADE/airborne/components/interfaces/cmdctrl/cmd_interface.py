#!/usr/bin/env python

import atexit
import os
import readline
import rlcompleter

from scl import generate_map
from arbiter_requests import Takeoff, Land, Move, RotateFixed, RotatePOI, Stop
from arbiter_interface import ArbiterInterface


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
_interface = ArbiterInterface(_socket)

def catch_ex(func):
   def wrapper(*__args,**__kw):
       try:
           return func(*__args,**__kw)
       except Exception, e:
           print str(e)
   return wrapper

@catch_ex
def takeoff(alt = None, speed = None):
   """takeoff"""
   _interface.execute(Takeoff(alt, speed)._req)


@catch_ex
def land(speed = None):
   """land at current position"""
   _interface.execute(Land(speed)._req)


@catch_ex
def move(pos, alt = None, speed = None, rel = None):
   """move to position"""
   _interface.execute(Move(pos, alt, speed, rel)._req)


@catch_ex
def rotate_fixed(yaw, rel = None, speed = None):
   """rotate UAV towards fixed angle"""
   _interface.execute(RotateFixed(yaw, rel, speed)._req)


@catch_ex
def rotate_poi(pos, rel = None, speed = None):
   """rotate UAV towards pos"""
   _interface.execute(RotatePOI(pos, rel, speed)._req)


@catch_ex
def stop():
   """stop UAV at current position"""
   _interface.execute(Stop()._req)


def help():
   for func in [takeoff, land, move, rotate_fixed, rotate_poi, stop]:
      print func.__name__ + '(' + ', '.join(func.func_code.co_varnames) + '):'
      print ' ' * 3 + func.__doc__ + '\n'

