from random import *

def vlen(a):
	return math.sqrt(a[0] * a[0] + a[1] * a[1])

def vsub(a, b):
	return (a[0] - b[0], a[1] - b[1])

def vadd(a, b):
	return (a[0] + b[0], a[1] + b[1])

def vnorm(a):
    il = 1.0/vlen(a)
    return (a[0] * il, a[1] * il)

def vscale(a, s):
    return (a[0] * s, a[1] * s)


DIRS = ((0, 1), (-1, 1), (-1, 0), (-1, -1), (0, -1), (1, -1), (1, 0), (1, 1))

dir = DIRS[0]
dirn = 0
hist = -91
sp = (0, 0)


class Controller:
   def decide(self, x, y, rssi, ts):
      global hist, dir, dirn, sp

      if rssi > hist:
         sp = vadd((x, y), dir)
      else:
         dirn = (dirn + 1) % len(DIRS)
         
         #rn = randrange(0, len(DIRS) - 1)
         #while  rn == dirn:
         #   rn = randrange(0, len(DIRS) - 1)
         #dirn = rn

         dir = DIRS[dirn]
         sp = vadd((x, y), dir)
         
      hist = rssi

      return sp[0], sp[1], 1
            
