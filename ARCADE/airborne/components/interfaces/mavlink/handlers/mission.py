
from threading import Thread
from mavlinkv10 import *


class MissionHandler(Thread):

   def __init__(self, dispatcher):
      Thread.__init__(self)
      self.dispatcher = dispatcher

   def _takeoff(self, arg):
      print 'takeoff'

   def _land(self, arg):
      print 'land'

   def run(self):
      cmd_map = {MAV_CMD_NAV_TAKEOFF: self._takeoff,
                 MAV_CMD_NAV_LAND: self._land}
      for e in self.dispatcher.generator('COMMAND_LONG'):
         try:
            cmd_map[e.command](e)
         except:
            print 'command not handled:', e

