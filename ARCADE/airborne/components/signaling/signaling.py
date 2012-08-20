#!/usr/bin/env python


from threading import Thread
from os import system
from time import sleep


class Beeper(Thread):

   def __init__(self, set_beep_gpio):
      Thread.__init__(self)
      self.code = []
      self.set_beep_gpio = set_beep_gpio
      self.daemon = True

   def run(self):
      while True:
         if len(self.code) == 0:
            sleep(0.1)
         else:
            for dt, beep in self.code:
               self.set_beep_gpio(beep)
               sleep(dt)



beeper = Beeper()
beeper.start()
sleep(3)
beeper.code = [(1, True), (2, False)]
sleep(5)
beeper.code = [(0.3, True), (0.1, False)]

def wall(self):
   system('echo "%s" | wall' % msg)

