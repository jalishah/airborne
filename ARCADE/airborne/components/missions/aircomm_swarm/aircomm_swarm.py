#!/usr/bin/env python

import sys
import zmq
from imms_sensor_data_pb2 import SensorData
from scl import generate_map
from time import sleep, time
from threading import Thread, Event
from core_pb2 import MonData
from icarus_interface import ICARUS_Client, ICARUS_MissionFactory
from gps_data_pb2 import GpsData
from numpy import array
from numpy.linalg import norm
from pyproj import Proj, transform
from random import random
from aircomm.packet.format import gpos
from aircomm_pb2 import AirComm
from misc import daemonize


def gps_add_meters((lat, lon), (dx, dy)):
   aeqd = Proj(proj = 'aeqd', lon_0 = lon, lat_0 = lat)
   meters = array(aeqd(lon, lat))
   meters += [dx, dy]
   lon, lat = aeqd(meters[0], meters[1], inverse = True)
   return lat, lon


def gps_meters_offset((lat1, lon1), (lat2, lon2)):
   aeqd = Proj(proj = 'aeqd', lon_0 = lon1, lat_0 = lat1)
   x1, y1 = aeqd(lon1, lat1)
   x2, y2 = aeqd(lon2, lat2)
   return array([x1 - x2, y1 - y2])


def sym_limit(x, limit):
   if x > limit:
      return limit
   elif x < -limit:
      return -limit
   return x




sep = 8
p = 0.25
cut = 1.0


def step(tv):
   while True:
      dist = norm(tv)
      if dist == 0.0:
         tv = array([random(), random()])
      else:
         break
   tv_norm = tv / dist
   ctrl = sym_limit(p * (sep - dist), cut)
   return ctrl * tv_norm


class GPS_Reader(Thread):
   
   def __init__(self, socket):
      Thread.__init__(self)
      self.daemon = True
      self.socket = socket

   def run(self):
      while True:
         data = GpsData()
         data.ParseFromString(self.socket.recv())
         self.data = data


def request(item):
   try:
      _client.execute(item)
   except Exception, e:
      print e


def main(name):
   sm = generate_map(name)
   in_socket = sm['aircomm_in']
   gps_reader = GPS_Reader(sm['gps'])
   gps_reader.start()
   _client = ICARUS_Client(sm['ctrl'])
   i = ICARUS_MissionFactory()
   while True:
      try:
         msg = AirComm()
         raw = in_socket.recv()
         msg.ParseFromString(raw)
         if msg:
            if msg.type == gpos.id:
               gps_nrf = array(gpos.unpack(msg.data)[0:2])
               gps_pos = array([gps_reader.data.lat, gps_reader.data.lon])
               print gps_pos, gps_nrf
               gps_target = array(gps_nrf)
               vec = gps_meters_offset(gps_pos, gps_target)
               ctrl = step(vec)
               print gps_pos, vec, norm(vec), ctrl
               request(i.move_xy(ctrl[0], ctrl[1]))
      except:
         sleep(1)


daemonize('nrf_swarm', main)

