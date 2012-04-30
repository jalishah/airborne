
from threading import Thread
from core_pb2 import MonData


class CoreMon(Thread):
 
   def __init__(self, socket):
      Thread.__init__(self)
      self.socket = socket
      self.data = MonData()

   def run(self):
      while True:
         data = self.socket.recv()
         self.data.ParseFromString(data)
         print self.data

