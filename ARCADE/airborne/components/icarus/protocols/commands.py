

class Takeoff:

   def __init__(self, **kwargs):
      if 'z' in kwargs:
         print 'z', kwargs['z']
      elif 'z_ground' in kwargs:
         print 'z_ground', kwargs['z_ground']
      else:
         raise AssertionError
      if 'speed' in kwargs:
         print 'speed', kwargs['speed']


class Move:

   def __init__(self, x, y, z, **kwargs):
      pass

class MoveGPS:

   def __init__(self, lon, lat, alt, kwargs):
      pass


