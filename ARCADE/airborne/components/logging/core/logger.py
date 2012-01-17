#!/usr/bin/env python

import zmq
import log_data_pb2
import sys
import signal
from scl import generate_map
from os.path import basename


def logdata_2_string(log_data):
   LOG_LEVEL_NAMES = ["ERROR", " WARN", " INFO", "DEBUG"];
   level_name = LOG_LEVEL_NAMES[log_data.level]
   file = basename(log_data.file)
   if log_data.details == 1:
      return "[%s] %s: %s" % (level_name, file, log_data.message)
   elif log_data.details == 2:
      return "[%s] %s,%d: %s" % (level_name, file, log_data.line, log_data.message)
   else:
      return "[%s] %s" % (level_name, log_data.message)

def signal_handler(signal, frame):
   sys.exit(0)


socket = generate_map('core_logger')['log']
signal.signal(signal.SIGINT, signal_handler)

while True:
   log_data = log_data_pb2.log_data()
   raw_data = socket.recv()
   log_data.ParseFromString(raw_data)
   print logdata_2_string(log_data)
