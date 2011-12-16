import zmq



class VirtAddrTable:

   def __init__(self):
      term_ip = '10.0.1.2'
      base_port = 6000
      self.uav_map = {}
      self.term_map = {}
      for i in range(0, 5):
         self.uav_map[i] = 'tcp://' + term_ip + ':%d' % (base_port + i)
         self.term_map[i] = 'tcp://*:%d' % (base_port + i)



class ZMQ_COM:

   def __init__(self, addr_tab):
      self.context = zmq.Context()
      self.addr_tab = addr_tab

   def connect(self, addr):
      self.socket = self.context.socket(zmq.REQ)
      self.socket.connect(self.addr_tab.uav_map[addr])
   
   def bind(self, addr):
      self.socket = self.context.socket(zmq.REP)
      self.socket.bind(self.addr_tab.term_map[addr])

   def send(self, data):
      self.socket.send(data)

   def receive(self):
      return self.socket.recv()

   def disconnect(self):
      self.socket.close()
      self.socket = None


