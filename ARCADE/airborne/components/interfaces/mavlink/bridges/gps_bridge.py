
from bridge import Bridge
from threading import Thread
from gps_data_pb2 import GpsData
from time import sleep
from struct import pack


class GpsBridge(Bridge):
   
   def __init__(self, socket_map, mav_iface, send_interval):
      Bridge.__init__(self, socket_map, mav_iface, send_interval)
      recv_thread = Thread(target = self._receive)
      send_thread = Thread(target = self._send)
      recv_thread.start()
      send_thread.start()

   def _receive(self):
      socket = self.socket_map['gps']
      while True:
         str = socket.recv()
         gps = GpsData()
         gps.ParseFromString(str)
         self.gps = gps

   def _send(self):
      while True:
         sleep(self.send_interval)
         try:
            gps = self.gps
         except AttributeError:
            continue
         self.mav_iface.send_gps_position(gps.fix, gps.lon, gps.lat, 
            gps.alt, gps.hdop, gps.vdop, gps.speed, gps.course, gps.sats)
         try:
            args = [''] * 5
            for satinfo in gps.satinfo:
               satinfo_pairs = [(0, satinfo.id), (1, satinfo.in_use),
                  (2, satinfo.elv), (3, satinfo.azimuth), (4, satinfo.sig)]
               for param_i, data in satinfo_pairs:
                  args[param_i] += pack('b', data)
            args = [len(gps.satinfo)] + args
            self.mav_iface.mavio.mav.gps_status_send(*args)
         except:
            pass
