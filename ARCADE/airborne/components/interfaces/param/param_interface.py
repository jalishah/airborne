#!/usr/bin/env python

import zmq
import param_pb2
import os
from zmq_ipc import generate_map


socket = generate_map('param_shell')['params']


def execute(request):
   socket.send(request.SerializeToString())
   recv_data = socket.recv()
   rep_data = param_pb2.ParamReply()
   try:
      rep_data.ParseFromString(recv_data)
      if rep_data.status != param_pb2.ParamReply.OK:
         print 'error'
      else:
         return rep_data
   except:
      print 'invalid reply from server'


def get(key = ""):
   """get value for key or a list of available keys if no argument was given"""
   request = param_pb2.ParamRequest()
   request.type = param_pb2.ParamRequest.GET
   request.key = key
   reply = execute(request)
   if reply:
      if key == "":
         print reply.desc
      else:
         print reply.val


def set(key, val):
   """set key to val"""
   request = param_pb2.ParamRequest()
   request.type = param_pb2.ParamRequest.SET
   request.key = key
   request.val = val
   execute(request)


def help():
   for func in [get, set]:
      print func.__name__ + '(' + ', '.join(func.func_code.co_varnames) + '):'
      print ' ' * 3 + func.__doc__ + '\n'


if __name__ == '__main__':
   print 'Welcome to the ARCADE parameters shell!'
   print 'enter help() for usage'

   import atexit
   import os
   import readline
   import rlcompleter
   
   historyPath = os.path.expanduser("~/.ARCADE_params_history")
   
   def save_history(historyPath=historyPath):
      import readline
      readline.write_history_file(historyPath)
   
   if os.path.exists(historyPath):
      readline.read_history_file(historyPath)
   
   atexit.register(save_history)

