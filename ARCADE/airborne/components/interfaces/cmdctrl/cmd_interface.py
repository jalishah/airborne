#!/usr/bin/env python

import atexit
import os
import readline
import rlcompleter

from scl import generate_map
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
   _interface.takeoff(alt, speed)


@catch_ex
def land(speed = None):
   """land at current position"""
   _interface.land(speed)


@catch_ex
def move(pos, alt = None, speed = None, rel = None):
   """move to position"""
   _interface.move(pos, alt, speed, rel)


@catch_ex
def rotate_fixed(yaw, rel = None, speed = None):
   """rotate UAV towards fixed angle"""
   _interface.rotate_fixed(yaw, rel, speed)


@catch_ex
def rotate_poi(pos, rel = None, speed = None):
   """rotate UAV towards pos"""
   _interface.rotate_poi(pos, rel, speed)


@catch_ex
def stop():
   """stop UAV at current position"""
   _interface.stop()

