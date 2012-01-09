#!/usr/bin/python
# GPS sensor and data publisher

from zmq import HWM
from time import sleep
from named_daemon import daemonize
from gps_data_pb2 import GpsData
from scl import generate_map
from nmea.gps import Gps
from nmea.serialport import SerialPort


def main(name):
   socket = generate_map(name)['gps']
   socket.setsockopt(HWM, 1)

   def new_fix_data(unused):
      sats = 0
      for sat in gps.satellitesInUse:
         if sat != '':
            sats += 1
      gps_data.fix = int(gps.fixType)
      gps_data.time = str(gps.fixTime)
      gps_data.lat = gps.position.lat
      gps_data.lon = gps.position.lng
      gps_data.alt = gps.altitude
      gps_data.sats = sats
      gps_data.hdop = gps.dop[1]
      if gps_data.sats >= 6 and gps_data.fix == 3:
         socket.send(gps_data.SerializeToString())

   gps_data = GpsData()
   gps = Gps(SerialPort("/dev/ttyACM0", 57600), callbacks = {'transit_update': new_fix_data})
   
   while True:
      try:
         gps.parse_data()
      except Exception, ex:
         print ex.__class__
         raise

main('gps_sensor')
daemonize('gps_sensor', main)

