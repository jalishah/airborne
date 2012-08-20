
import zmq
from gps_data_pb2 import GpsData
from networks_pb2 import Measurement
from measurement_pb2 import CombinedMeasurement
from threading import Thread, Event
from time import sleep
from os import system
from mission_pb2 import MissionMessage
from icarus_interface import ICARUS_MissionFactory, ICARUS_SynClient


class _GPS_Reader(Thread):
   
   """
   reads GPS network information
   """

   def __init__(self, socket):
      Thread.__init__(self)
      self.daemon = True
      self.socket = socket
      self.event = Event()
      self.cancel = False

   def term(self):
      self.cancel = True
      self.join()

   def run(self):
      while not self.cancel:
         data = GpsData()
         data.ParseFromString(self.socket.recv())
         self.data = data
         self.event.set()


class _MeasureReader(Thread):

   """
   reads WiFi network information
   """

   def __init__(self, socket):
      Thread.__init__(self)
      self.daemon = True
      self.socket = socket
      self.event = Event()
      self.cancel = False
   
   def term(self):
      self.cancel = True
      self.join()

   def run(self):
      while not self.cancel:
         data = Measurement()
         raw_data = self.socket.recv()
         data.ParseFromString(raw_data)
         self.data = data
         self.event.set()


class _CombinedPublisher(Thread):

   """
   - acquires individual measurements via gps_reader and measure_reader
   - sends combined measurement to socket
   """

   def __init__(self, socket, gps_reader, measure_reader):
      Thread.__init__(self)
      self.measure_reader = measure_reader
      self.gps_reader = gps_reader
      self.daemon = True
      self.socket = socket
      self.cancel = False
   
   def term(self):
      self.cancel = True
      self.join()

   def run(self):
      # wait for first gps measurement:
      self.gps_reader.event.wait()
      while not self.cancel:
         # wait for new network measurement:
         self.measure_reader.event.wait()
         self.measure_reader.event.clear()
         # get data from both data sources:
         measure = self.measure_reader.data
         gps = self.gps_reader.data
         # create measurement to publish
         cm = CombinedMeasurement()
         cm.type = 10
         cm.lat = gps.lat
         cm.lon = gps.lon
         cm.mac = measure.mac
         cm.rssi = measure.rssi
         self.socket.send(cm.SerializeToString())



class LocMission(Thread):

   """
   - manages a measurement publisher
   - executes a configurable zigzag mission
   """

   def __init__(self, map, req):
      Thread.__init__(self)
      #system('svctrl --start wifi_sensor > /dev/null')
      self.gps_reader = _GPS_Reader(map['gps'])
      self.measure_reader = _MeasureReader(map['net'])
      self.combined_publisher = _CombinedPublisher(map['pub_server'], self.gps_reader, self.measure_reader)
      self.map = map
      self.req = req
      self.factory = ICARUS_MissionFactory()
      self.client = ICARUS_SynClient(map['ctrl'], map['state'])

   def run(self):
      self.gps_reader.start()
      self.measure_reader.start()
      self.combined_publisher.start()
      for x, y in self._zigzag_gen(0.0, 0.0, 8.0, 20.0, 10):
         self.client.execute(self.factory.move_xy(x, y))
      self.combined_publisher.term()
      self.measure_reader.term()
      self.gps_reader.term()
      self.req.status = MissionMessage.DONE
      self.map['pub_server'].send(self.req.SerializeToString())


   def _zigzag_gen(self, x, y, width, height, breaks):
      for i in range(breaks):
         yield x + width / 2.0, y
         y += (height / breaks) / 2.0
         yield x - width / 2.0, y
         y += (height / breaks) / 2.0
      yield width / 2.0, height

