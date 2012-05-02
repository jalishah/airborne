

#
# file: geomath.py
# purpose: various geodetic math functions
#
# author: Tobias Simon, Ilmenau University of Technology
#


from math import sin, cos, atan2, pi


def deg2rad(deg):
   return deg / 180.0 * pi


def rad2deg(rad):
   return rad * 180.0 / pi


def bearing(lat1, lon1, lat2, lon2):
   dLon = lon2 - lon1
   y = sin(dLon) * cos(lat2)
   x = cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2) * cos(dLon)
   return atan2(y, x)


def bearing_deg(*args): # lat1, lon1, lat2, lon2
   args = map(deg2rad, args)
   return rad2deg(bearing(*args))


def lineq_n(x1, y1, x2, y2):
   n = y1 - (y2 - y1) / (x2 - x1) * x1



if __name__ == '__main__':

   import unittest

   class TestGeoMath(unittest.TestCase):

      def test_bearing(self):
         self.assertEqual(bearing_deg(50.0, 5.0, 55.0, 7.0), 12.896821904227854)
         self.assertEqual(bearing_deg(50.0, 5.0, 50.0, 7.0), 89.23392341814612)

   unittest.main()

