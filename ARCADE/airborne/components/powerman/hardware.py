

from threading import Thread
from time import sleep



class ADC:

   def __init__(self, adc_id):
      self.path = '/sys/class/hwmon/hwmon0/device/in%d_input' % adc_id

   def read(self):
      return int(file(self.path).read())


class GPIO_Bank:

   def __init__(self, bus, dev):
      self.bus = bus
      self.dev = int(dev)
      self.state = 0

   def set_gpio(self, id, state):
      id = int(id)
      if id < 0 or id > 7:
         raise ValueError('expected id to be in [0 ... 7]')
      state = bool(state)
      if state:
         self.state |= 1 << id;
      else:
         self.state &= ~(1 << id)
      self.bus.write_byte_data(self.dev, 1, self.state)


