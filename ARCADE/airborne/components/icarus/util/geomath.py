

#
# file: geomath.py
# purpose: various geodetic math functions
#
# author: Tobias Simon, Ilmenau University of Technology
# calc_rad, earth_distance and meter_offset code taken from:
# http://code.google.com/p/gmapcatcher
#


from math import sin, cos, acos, atan2, pi, fmod, hypot, asin
from pyproj import Proj, transform


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


def calc_rad(lat):
    "Radius of curvature in meters at specified latitude."
    a = 6378.137
    e2 = 0.081082 * 0.081082
    # the radius of curvature of an ellipsoidal Earth in the plane of a
    # meridian of latitude is given by
    #
    # R' = a * (1 - e^2) / (1 - e^2 * (sin(lat))^2)^(3/2)
    #
    # where a is the equatorial radius,
    # b is the polar radius, and
    # e is the eccentricity of the ellipsoid = sqrt(1 - b^2/a^2)
    #
    # a = 6378 km (3963 mi) Equatorial radius (surface to center distance)
    # b = 6356.752 km (3950 mi) Polar radius (surface to center distance)
    # e = 0.081082 Eccentricity
    sc = sin(deg2rad(lat))
    x = a * (1.0 - e2)
    z = 1.0 - e2 * sc * sc
    y = pow(z, 1.5)
    r = x / y

    r = r * 1000.0   # Convert to meters
    return r


def earth_distance((lat1, lon1), (lat2, lon2)):
    "Distance in meters between two points specified in degrees."
    x1 = calc_rad(lat1) * cos(deg2rad(lon1)) * sin(deg2rad(90-lat1))
    x2 = calc_rad(lat2) * cos(deg2rad(lon2)) * sin(deg2rad(90-lat2))
    y1 = calc_rad(lat1) * sin(deg2rad(lon1)) * sin(deg2rad(90-lat1))
    y2 = calc_rad(lat2) * sin(deg2rad(lon2)) * sin(deg2rad(90-lat2))
    z1 = calc_rad(lat1) * cos(deg2rad(90-lat1))
    z2 = calc_rad(lat2) * cos(deg2rad(90-lat2))
    a = (x1*x2 + y1*y2 + z1*z2)/pow(calc_rad((lat1+lat2)/2), 2)
    # a should be in [1, -1] but can sometimes fall outside it by
    # a very small amount due to rounding errors in the preceding
    # calculations (this is prone to happen when the argument points
    # are very close together).  Thus we constrain it here.
    if abs(a) > 1: a = 1
    elif a < -1: a = -1
    return calc_rad((lat1 + lat2) / 2) * acos(a)


def meter_offset((lat1, lon1), (lat2, lon2)):
    "Return offset in meters of second arg from first."
    dy = earth_distance((lat1, lon1), (lat1, lon2))
    dx = earth_distance((lat1, lon1), (lat2, lon1))
    if lat1 < lat2: dx *= -1
    if lon1 < lon2: dy *= -1
    return (dx, dy)


class GPS_Shifter:
   '''
   takes initial gps position, adds relative shift in meters
   and transforms it back into a gps position
   '''

   def __init__(self, lat, lon):
      self.gps = Proj(proj='latlong', datum='WGS84')
      self.aeqd = Proj(proj='aeqd')
      self.start_lat = lat
      self.start_lon = lon

   def calc(self, dx, dy):
      aey, aex = transform(self.gps, self.aeqd,
                           self.start_lat, self.start_lon)
      aey += dy
      aex += dx
      return transform(self.aeqd, self.gps, aey, aex)



if __name__ == '__main__':

   import unittest

   class TestGeoMath(unittest.TestCase):

      def test_bearing(self):
         self.assertEqual(bearing_deg(50.0, 5.0, 55.0, 7.0), 12.896821904227854)
         self.assertEqual(bearing_deg(50.0, 5.0, 50.0, 7.0), 89.23392341814612)

      def test_earth_distance(self):
         self.assertEqual(earth_distance((0, 0), (0, 0)), 0.0)
         self.assertEqual(meter_offset((0, 0), (0, 1)), (0.0, -110587.64409758021))

      def test_shifter(self):
         shifter = GPS_Shifter(50.0, 10.0)
         self.assertEqual(shifter.calc(1000000, 0), (51.292293466969085, 17.86742449270132))

   unittest.main()

