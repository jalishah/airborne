
from time import time


class Hysteresis:

   def __init__(self, timeout):
      self.timeout = timeout
      self.start_time = None

   def set(self):
      if self.start_time == None:
         self.start_time = time()
      elif self.start_time + self.timeout < time():
         return True
      return False
   
   def reset(self):
      self.start_time = None

