#!/usr/bin/env python

<<<<<<< HEAD
from aircomm.interface import Interface as ACI
from scl import generate_map
from time import sleep
from threading import Thread
from opcd_interface import OPCD_Interface
from aircomm_pb2 import AirComm
from misc import daemonize


class ACIReader(Thread):

   def __init__(self, aci, scl_socket):
      Thread.__init__(self)
      self.daemon = True
      self.aci = aci
      self.scl_socket = scl_socket
=======
from aircomm.packet.dencode import Packet, BCAST
from aircomm.packet.types import POS, pos_pack, pos_unpack
from aircomm.interface import Interface
from time import sleep
from scl import generate_map
from time import sleep, time
from threading import Thread, Lock
from core_pb2 import MonData
from random import random


class MonReader(Thread):

   def __init__(self, socket):
      Thread.__init__(self)
      self.daemon = True
      self.socket = socket

   def run(self):
      while True:
         data = MonData()
         data.ParseFromString(self.socket.recv())
         self.data = data


l = Lock()

class Reader(Thread):

   def __init__(self, interf):
      Thread.__init__(self)
      self.daemon = True
      self.interf = interf
>>>>>>> 277515ceb5d948fffa8bf3a6188921fe5af21fea

   def run(self):
      s = 0
      next_seq = 0
      while True:
         sleep(0.01)
<<<<<<< HEAD
         try:
            msg = self.aci.receive()
            if msg:
               #   if msg.dst == i.addr:
               #      handle(msg)
               #   elif msg.dst == BCAST:
               #      handle(msg)
               #      bcast(msg)
               #   else:
               #      bcast(msg)
               if msg.seq != next_seq:
                  c = abs(next_seq - msg.seq)
                  if c < 127:
                     s += c
                  #print 'lost %d packets' % s
               next_seq = (msg.seq + 1) % 256
               pb_msg = AirComm()
               pb_msg.addr = msg.src
               pb_msg.type = msg.type
               pb_msg.data = msg.data
               self.scl_socket.send(pb_msg.SerializeToString())
         except:
            pass

def main(name):
   sm = generate_map(name)
   opcd = OPCD_Interface(sm['opcd_ctrl'], name)
   sys_id = opcd.get('id')
   out_socket = sm['out']
   in_socket = sm['in']

   aci = ACI(sys_id, '/dev/ttyACM0')
   acr = ACIReader(aci, out_socket)
   acr.start()

   # read from SCL in socket and send data via NRF
   while True:
      try:
         msg = AirComm()
         raw = self.in_socket.recv()
         msg.ParseFromString(raw)
         aci.send(msg.addr, msg.type, msg.data)
      except:
         sleep(0.1)

daemonize('aircomm', main)
=======
         l.acquire()
         msg = self.interf.receive()
         l.release()
         if msg:
            if msg.seq != next_seq:
               c = abs(next_seq - msg.seq)
               if c < 127:
                  s += c
               print 'lost %d packets' % s
            next_seq = (msg.seq + 1) % 256
            if msg.type == POS:
               print msg.src, msg.dst, msg.seq, pos_unpack(msg.data)

sm = generate_map('aircomm')
mon_reader = MonReader(sm['core_mon'])
mon_reader.start()

def handle(msg):
   print msg

def bcast(msg):
   i.send(msg.dst, msg.type, msg.data)


i = Interface(1, '/dev/ttyACM0')
reader = Reader(i)
reader.start()
while True:
   sleep(0.1 + 0.1 * random())
   #l.acquire()
   #i.send(BCAST, POS, pos_pack(mon_reader.data.x, mon_reader.data.y))
   #l.release()
   #msg = i.receive()
   #print mon_reader.data
   #if msg:
   #   if msg.dst == i.addr:
   #      handle(msg)
   #   elif msg.dst == BCAST:
   #      handle(msg)
   #      bcast(msg)
   #   else:
   #      bcast(msg)


>>>>>>> 277515ceb5d948fffa8bf3a6188921fe5af21fea
