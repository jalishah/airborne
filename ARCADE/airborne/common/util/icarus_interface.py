

# This file is part of ARCADE's ICARUS component.
# ICARUS stands for: (I)ntelligent (C)ommand (A)rbitration and (R)eaction on (U)nforeseen (S)ituations
#
# The purpose of ICARUS is not only the execution of a given command, but to:
# - perform contraint analysis on parameters: checking speed against actuator limits, checking position against battery capacity constraints
# - manage possible auto-landing sites
# - monitor health and react on different low battery states


class ICARUS_Error(Exception):
   pass



class ICARUS_Interface:

   '''
   human- and machine-friendly interface to the UAV's ICARUS system
   '''

   def __init__(self, backend):
      self.backend = backend
   
   
   def takeoff(self, **kwargs):
      '''
      take-off

      keyword arguments:
         - z: take-off altitude
         - glob: indicates if z should be interpreted global or local
         - speed: take-off speed. Overrides z speed until take-off is complete
      '''
      z = self._get_and_cast(kwargs, 'z', float, None)
      if z == None and 'glob' in kwargs:
         raise AssertionError('glob provided without z')
      glob = self._get_glob(kwargs)
      speed = self._get_speed(kwargs)
      return self.backend.takeoff(z, glob, speed)


   def land(self, speed = None):
      '''
      land
      
      arguments:
         - speed: landing speed. Overrides z speed until landing is complete
      '''
      speed = self._cast(speed, float)
      return self.backend.land(speed)



   def stop(self):
      '''
      stop the system when moving
      '''
      return self.backend.stop()



   def move_x(self, x, **kwargs):
      '''
      move local in x direction

      arguments:
         x value in meters
      keyword arguments:
         rel: indicates, if the movement is incremental (default: False)
         speed: movement speed
         block: indicates if the interface should wait for completion (default: True)
      '''
      self._set_glob(kwargs, False)
      return self.move((x, None, None), **kwargs)


   def move_y(self, y, **kwargs):
      '''
      move local in y direction

      arguments:
         y value in meters
      keyword arguments:
         rel: indicates, if the movement is incremental (default: False)
         speed: movement speed
         block: indicates if the interface should wait for completion (default: True)
      '''
      self._set_glob(kwargs, False)
      return self.move((None, y, None), **kwargs)


   def move_z(self, z, **kwargs):
      '''
      move local in z direction

      arguments:
         z value in meters
      keyword arguments:
         rel: indicates, if the movement is incremental (default: False)
         speed: movement speed
         block: indicates if the interface should wait for completion (default: True)
      '''
      self._set_glob(kwargs, False)
      return self.move((None, None, z), **kwargs)


   def move_xy(self, x, y, **kwargs):
      '''
      move local in x and y direction

      arguments:
         x value in meters
         y value in meters
      keyword arguments:
         rel: indicates, if the movement is incremental (default: False)
         speed: movement speed
         block: indicates if the interface should wait for completion (default: True)
      '''
      self._set_glob(kwargs, False)
      return self.move((x, y, None), **kwargs)


   def move_xyz(self, x, y, z, **kwargs):
      '''
      move local in x, y and z direction

      arguments:
         x value in meters
         y value in meters
         z value in meters
      keyword arguments:
         rel: indicates, if the movement is incremental (default: False)
         speed: movement speed
         block: indicates if the interface should wait for completion (default: True)
      '''
      self._set_glob(kwargs, False)
      return self.move((x, y, z), **kwargs)


   def move_lon(self, lon, **kwargs):
      '''
      move to GPS longitude

      arguments:
         lon: longitude in radians
      keyword arguments:
         rel: indicates, if the movement is incremental (default: False)
         speed: movement speed
         block: indicates if the interface should wait for completion (default: True)
      '''
      self._set_glob(kwargs, True)
      return self.move((lon, None, None), **kwargs)


   def move_lat(self, lat, **kwargs):
      '''
      move to GPS latitude

      arguments:
         lat: latitude in radians
      keyword arguments:
         rel: indicates, if the movement is incremental (default: False)
         speed: movement speed
         block: indicates if the interface should wait for completion (default: True)
      '''
      self._set_glob(kwargs, True)
      return self.move((None, lat, None), **kwargs)


   def move_alt(self, alt, **kwargs):
      '''
      move to GPS altitude (altitude above mean sea level (MSL))

      arguments:
         alt: altitude in meters above MSL
      keyword arguments:
         rel: indicates, if the movement is incremental (default: False)
         speed: movement speed
         block: indicates if the interface should wait for completion (default: True)
      '''
      self._set_glob(kwargs, True)
      return self.move((None, None, alt), **kwargs)


   def move_lon_lat(self, lon, lat, **kwargs):
      '''
      move to GPS longitude, latitude

      arguments:
         lon: longitude in radians
         lat: latitude in radians
      keyword arguments:
         rel: indicates, if the movement is incremental (default: False)
         speed: movement speed
         block: indicates if the interface should wait for completion (default: True)
      '''
      self._set_glob(kwargs, True)
      return self.move((lon, lat, None), **kwargs)


   def move_gps(self, lon, lat, alt, **kwargs):
      '''
      move to GPS position longitude, latitude, altitude

      arguments:
         lon: longitude in radians
         lat: latitude in radians
         alt: altitude in meters above MSL
      keyword arguments:
         rel: indicates, if the movement is incremental (default: False)
         speed: movement speed (ground speed)
         block: indicates if the interface should wait for completion (default: True)
      '''
      self._set_glob(kwargs, True)
      return self.move((lon, lat, alt), **kwargs)


   def move(self, pos, **kwargs):
      '''
      move generic

      arguments:
         pos: tuple containing 3d coordinate (absolute / relative, global / local)
      keyword arguments:
         glob: indicates if pos should be interpreted global (True) or local (default: True)
         rel: indicates, if the movement is incremental (default: False)
         speed: movement speed (ground speed)
         block: indicates if the interface should wait for completion (default: True)
      '''
      pos = tuple(map(lambda x: self._cast(x, float), pos))
      glob = self._get_glob(kwargs)
      rel = self._get_rel(kwargs)
      speed = self._get_speed(kwargs)
      block = self._get_block(kwargs)
      return self.backend.move(pos, glob, rel, speed, block)


   # private utility methods:


   def _cast(self, val, cast):
      if val is not None:
         try:
            return cast(val)
         except:
            raise TypeError('cannot cast %s to type: %s' % (str(val), str(cast)))


   def _get_and_cast(self, kwargs, name, cast, default):
      if name in kwargs:
         return self._cast(kwargs[name], cast)
      return default


   def _get_glob(self, kwargs):
      return self._get_and_cast(kwargs, 'glob', bool, False)


   def _get_rel(self, kwargs):
      return self._get_and_cast(kwargs, 'rel', bool, False)


   def _get_speed(self, kwargs):
      return self._get_and_cast(kwargs, 'speed', float, None)


   def _get_block(self, kwargs):
      return self._get_and_cast(kwargs, 'block', bool, True)


   def _set_glob(self, kwargs, val):
      assert 'glob' not in kwargs
      kwargs['glob'] = val

   
   def _set_rel(self, kwargs, val):
      assert 'rel' not in kwargs
      kwargs['rel'] = val




if __name__ == '__main__':   
 
   import unittest


   class TestBackend:

      def takeoff(self, z, glob, speed):
         return z, glob, speed

      def land(self, speed):
         return speed
   
      def stop(self):
         pass

      def move(self, pos, glob, rel, speed, block):
         return pos, glob, rel, speed, block


   class TestTakeoff(unittest.TestCase):

      def __init__(self, arg):
         unittest.TestCase.__init__(self, arg)
         backend = TestBackend()
         self.mi = MissionInterface(backend)

      def test_takeoff(self):
         self.assertEqual(self.mi.takeoff(), (None, False, None))
         self.assertEqual(self.mi.takeoff(z = 1.0), (1.0, False, None))
         self.assertEqual(self.mi.takeoff(speed = 10), (None, False, 10.0))
         self.assertRaises(AssertionError, self.mi.takeoff, glob = False)
         self.assertEqual(self.mi.takeoff(z = 1.0), (1.0, False, None))
         self.assertEqual(self.mi.takeoff(z = 1.0), (1.0, False, None))
         self.assertEqual(self.mi.takeoff(z = 1.0, speed = 2.0), (1.0, False, 2.0))
         self.assertEqual(self.mi.takeoff(z = 7.0, speed = 3.0), (7.0, False, 3.0))
         self.assertRaises(TypeError, self.mi.takeoff, z = 'text')
         self.assertRaises(TypeError, self.mi.takeoff, z = 'text')
         self.assertRaises(TypeError, self.mi.takeoff, speed = 'text')

      def test_land(self):
         self.assertEqual(self.mi.land(), None)
         self.assertEqual(self.mi.land(20), 20.0)
         self.assertRaises(TypeError, self.mi.land, 'text')
 
      def test_stop(self):
         self.assertEqual(self.mi.stop(), None)

      def test_move(self):
         self.assertEqual(self.mi.move_x(1.0), ((1.0, None, None), False, False, None, True))
         self.assertEqual(self.mi.move_y('1'), ((None, 1.0, None), False, False, None, True))
         self.assertEqual(self.mi.move_z(1), ((None, None, 1.0), False, False, None, True))
         self.assertEqual(self.mi.move_xy(1.0, 2.0), ((1.0, 2.0, None), False, False, None, True))
         self.assertEqual(self.mi.move_xyz(1.0, 2.0, 3.0), ((1.0, 2.0, 3.0), False, False, None, True))
         self.assertEqual(self.mi.move_xyz(1.0, 2.0, 3.0, speed = 2), ((1.0, 2.0, 3.0), False, False, 2.0, True))
         self.assertEqual(self.mi.move_xyz(1.0, 2.0, 3.0, rel = True, speed = 2), ((1.0, 2.0, 3.0), False, True, 2.0, True))
         self.assertEqual(self.mi.move_xyz(1.0, 2.0, 3.0, rel = True, speed = 2, block = False), ((1.0, 2.0, 3.0), False, True, 2.0, False))
         self.assertEqual(self.mi.move_lon(1.0, speed = 2, block = False), ((1.0, None, None), True, False, 2.0, False))
         self.assertEqual(self.mi.move_lat(1.0), ((None, 1.0, None), True, False, None, True))
         self.assertEqual(self.mi.move_lon_lat(1.0, 2.0), ((1.0, 2.0, None), True, False, None, True))
         self.assertEqual(self.mi.move_gps(1.0, 2.0, 3.0), ((1.0, 2.0, 3.0), True, False, None, True))
         self.assertRaises(TypeError, self.mi.move_gps, 'text')
         self.assertRaises(TypeError, self.mi.move_gps, 1.0, 'text')
         self.assertRaises(TypeError, self.mi.move_gps, 1.0, 2.0, 'text')
         self.assertRaises(TypeError, self.mi.move_gps, 1.0, 2.0, 3.0, speed = 'text')
         self.assertRaises(TypeError, self.mi.move_gps, 1.0, 2.0, 3.0, speed = 'text')
         self.assertRaises(AssertionError, self.mi.move_gps, 1.0, 2.0, 3.0, speed = 'text', glob = True)

   unittest.main()



def mission():
   m.takeoff(z = 10, glob = True)
   for x, y in [(0, 1), (1, 1), (1, 0), (0, 0)]:
      scale = 10
      m.move_xy(scale * x, scale * y)
   m.land()

