
from threading import Thread
from os import system
from mission_pb2 import MissionMessage
from time import sleep


class PlaceMission(Thread):

   """
   performs placement mission
   """

   def __init__(self, map, gpos, req, land = False):
      Thread.__init__(self)
      self.map = map
      self.gpos = gpos
      self.land = land
      self.req = req

   def run(self):
      system('svctrl --stop wifi_sensor > /dev/null')
      print 'starting placement'
      sleep(3)
      print 'placement done'
      self.req.status = MissionMessage.DONE
      self.map['pub_server'].send(self.req.SerializeToString())

