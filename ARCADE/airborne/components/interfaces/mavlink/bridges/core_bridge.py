
from bridge import Bridge
from threading import Thread
from monitor_data_pb2 import CoreMonData
from time import sleep
from math import sqrt


class CoreBridge(Bridge):
 
   def __init__(self, socket_map, mav_iface, send_interval):
      Bridge.__init__(self, socket_map, mav_iface, send_interval)
      recv_thread = Thread(target = self._receive)
      send_thread = Thread(target = self._send)
      recv_thread.start()
      send_thread.start()

   def _receive(self):
      mon = CoreMonData()
      socket = self.socket_map['core_mon']
      while True:
         str = socket.recv()
         mon.ParseFromString(str)
         self.mon = mon

   def _send(self):
      while True:
         sleep(self.send_interval)
         try:
            mon = self.mon
            self.mav_iface.send_local_position(mon.x, mon.y, mon.z,
               mon.x_speed, mon.y_speed, mon.z_speed)
            self.mav_iface.send_attitude(mon.roll, mon.pitch, mon.yaw,
               mon.roll_speed, mon.pitch_speed, mon.yaw_speed)
            ground_speed = sqrt(mon.x_speed ** 2, mon.y_speed ** 2)
            airspeed = 0.0 # TODO: fix me
            throttle = 0.5 # todo: fix me
            self.mav_iface.mavio.mav.vfr_hud_send(airspeed, ground_speed,
               mon.yaw, throttle, mon.z, mon.z_speed)
         except:
            pass

