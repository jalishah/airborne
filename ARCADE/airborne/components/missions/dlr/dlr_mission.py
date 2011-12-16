
from threading import Thread, Event
from zmq_ipc import generate_map
from mission import MissionExecutor
from math import sin, cos, atan2, pi
from gps_data_pb2 import GpsData


def calc_bearing(pos2, pos1):
   lon2, lat2 = pos2
   lon1, lat1 = pos1
   d_lon = lon2 - lon1
   y = sin(d_lon) * cos(lat2)
   x = cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2) * cos(d_lon)
   return atan2(y, x) % (2 * pi)


class GPSReader(Thread):

   def __init__(self, socket):
      Thread.__init__(self)
      self.daemon = True
      self.socket = socket
      self.event = Event()

   def run(self):
      while True:
         data = GpsData()
         data.ParseFromString(self.socket.recv())
         self.data = data
         self.event.set()

   def pos(self):
      self.event.wait()
      return self.data.lon, self.data.lat


gps_reader = GPSReader(generate_map('dlr_logger')['gps'])
gps_reader.start()


def dlr_mission():
   gs_pos = 50.0, 10.0
   print 'taking off'
   yield 'takeoff', 4.5
   own_pos = gps_reader.pos()
   bearing = calc_bearing(gs_pos, own_pos)
   print 'rotating to', bearing
   yield 'rotate', bearing



executor = MissionExecutor(generate_map('ferry_ctrl'))
executor.execute(dlr_mission())

