

from threading import Thread


class MissionManager:

   '''
   starts and keeps track
   '''

   def __init__(self):
      self.mission = EmptyMission()
      self.mission.start()

   def start_mission(self, mission, blocking = False):
      print 'new mission', mission.__class__.__name__
      print 'waiting for mission', self.mission.__class__.__name__
      #if blocking and self.mission.is_alive():
      #   raise RuntimeError
      self.mission.join()
      self.mission = mission
      self.mission.start()


class EmptyMission(Thread):
   pass

