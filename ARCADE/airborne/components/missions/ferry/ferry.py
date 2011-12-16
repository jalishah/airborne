
import time
from ferry_term_addr import VirtAddrTable, ZMQ_COM
from zmq_ipc import generate_map
from ferry_pb2 import Envelope, Message
from mission import MissionExecutor
from hop_selector import *


class Ferry:

   def __init__(self, hop_selector):
      self.term_index = 0
      self.term_map = {0: (-10,  0),
                       1: (-10, 10),
                       2: ( 10, 10), 
                       3: ( 10,  0), 
                       4: (  5,  0)}
      self.hop_selector = hop_selector
      self.hop_selector.register(self)
      self.messages = []
      self.delays = []
      self.buf_sizes = []

   def send_push_request(self, com, message):
      envelope = Envelope()
      envelope.type = Envelope.REQ_PUSH
      envelope.message.dest = message.dest
      envelope.message.src = message.src
      envelope.message.payload = message.payload
      envelope.message.timestamp = message.timestamp
      com.send(envelope.SerializeToString())

   def send_pull_request(self, com):
      envelope = Envelope()
      envelope.type = Envelope.REQ_PULL
      com.send(envelope.SerializeToString())

   def transfer_messages(self, addr):
      # open comm link to terminal:
      com = ZMQ_COM(VirtAddrTable())
      com.connect(addr)

      # pull all messages from terminal:
      count = 0
      while True:
         self.send_pull_request(com)
         envelope = Envelope()
         envelope.ParseFromString(com.receive())
         if envelope.type == Envelope.REP_ERR:
            break # terminal buffer is empty
         envelope.message.timestamp = time.time()
         self.messages.append(envelope.message)
         count += 1
      print 'pulled', count, 'messages'
      self.buf_sizes.append((addr, count)) # reflects terminal buffer size

      # push all messages addressed to terminal:
      count = 0
      for message in self.messages:
         if message.dest == addr:
            self.messages.remove(message)
            self.send_push_request(com, message)
            envelope = Envelope()
            envelope.ParseFromString(com.receive())
            assert envelope.type != Envelope.REP_ERR
            self.delays.append((message.dest, message.src, time.time() - message.timestamp))
            count += 1
      print 'pushed', count, 'messages'

      # close comm link:
      com.disconnect()
   


class FerryMission:

   def __init__(self):
      hop_selector = HistogramHopSelector()
      self.ferry = Ferry(hop_selector)
      self.run_time = 60.0 * 10.0

   def generator(self):
      yield ('takeoff',)
      start_time = time.time()
      while True:
         if time.time() - start_time > self.run_time:
            print 'time limit exceeded, stopping experiment'
            yield 'moverel', 0, 0
            time.sleep(5.0)
            yield 'land',
            break
         i, pos = self.ferry.hop_selector.get_next_hop()
         print 'moving to terminal', i, 'at pos', pos
         yield 'moverel', pos[0], pos[1]
         print 'arrived at terminal', i
         print 'exchanging messages with terminal %d' % i
         self.ferry.transfer_messages(i)
      # write stats:
      delays_file = open('delays.txt', 'w')
      for delay in self.ferry.delays:
         delays_file.write('%d %d %.2f\n' % delay)
      delays_file.close()
      buf_sizes_file = open('buf_sizes.txt', 'w')
      for buf_size in self.ferry.buf_sizes:
         buf_sizes_file.write('%d %d\n' % buf_size)
      buf_sizes_file.close()



executor = MissionExecutor(generate_map('ferry_ctrl'))
executor.execute(FerryMission().generator())

