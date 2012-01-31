
# Generator-based Dispatcher


from Queue import Queue
from threading import Thread, Event
from time import sleep


class GenDisp(Thread):

   '''
   GenDisp: Generator-based Dispatcher

   The dispatcher is a thread, reading pairs from a
   source until the read function returns None.
   For each pair, the first element (type) defines,
   which generator the second element is passed to.
   When a thread reads from a generator,
   it is woken up when new data delegated to it is available.
   Otherwise, the thread is sleeping.
   
   In comparison to mostly used "handlers",
   the consuming thread can be programmed in a more natural way.
   '''


   def __init__(self, source, debug = False):
      Thread.__init__(self)
      self.source = source
      self.queues = {}
      self.debug = debug


   def iter(self, type):
      if self.debug:
         print 'message reader registered for type', type
      q = Queue()
      self.queues[type] = q
      while True:
         data = q.get()
         if data:
            yield data
            q.task_done()
         else:
            return


   def run(self):
      while True:
         try:
            type, data = self.source.read_pair()
            if self.debug:
               print 'dispatcher is delegating:', type, data
         except TypeError:
            for queue in self.queues.values():
               queue.put(None)
            break
         try:
            self.queues[type].put(data)
         except KeyError, e:
            if self.debug:
               print 'data of type %s not handled' % str(type)


if __name__ == '__main__':

   class TestSource:

      def __init__(self, comm, parser):
         self.comm = comm
         self.parser = parser

      def read_pair(self):
         for i in range(2):
            byte = self.comm.read_byte()
            if byte is None:
               return None
            try:
               type, data = self.parser.parse_byte(byte)
            except:
               pass
         return type, data


   class TestComm:

      def __init__(self):
         self.bytes = ['a', 1, 'b', 2, 'a', 6, 'b', 255]

      def read_byte(self):
         try:
            byte = self.bytes[0]
            self.bytes = self.bytes[1:]
            sleep(1.0)
            return byte
         except:
            pass
         

   class TestParser:

      def __init__(self):
         self.state = 0

      def parse_byte(self, byte):
         if self.state == 1:
            self.state = 0
            return self.type, byte
         else:
            self.type = byte
         self.state += 1


   source = TestSource(TestComm(), TestParser())
   disp = GenDisp(source, True)
   disp.start()

   def handle_a():
      for e in disp.iter('a'):
         print 'handle_a woke up with data:', e

   def handle_b():
      for e in disp.iter('b'):
         print 'handle_b woke up with data:', e

   ta = Thread(target = handle_a)
   ta.start()

   tb = Thread(target = handle_b)
   tb.start()

