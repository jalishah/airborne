#!/usr/bin/env python


#
# file: powerman.py (POWER MANager)
# purpose: power management
# responsibilities:
#   - monitoring system power
#   - estimating battery state of charge (SOC)
#   - predicting remaining battery lifetime
#   - managing main power and lights depending on the provided system power profile
# author: Tobias Simon, Ilmenau University of Technology
#


# Python standard lib imports:
from time import sleep
from threading import Thread, Timer
from smbus import SMBus

# ARCADE imports:
from power_pb2 import PowerState, PowerRequest, PowerReply, PowerRequest, OK, E_SYNTAX, E_POWER, STANDING
from scl import generate_map
from named_daemon import daemonize
from opcd_interface import OPCD_Interface

# component imports:
from hardware import ADC, GPIO_Bank


class PowerMan:

   def __init__(self, name):
      # initialized and load config:
      map = generate_map(name)
      self.ctrl_socket = map['ctrl']
      self.monitor_socket = map['mon']
      self.opcd = OPCD_Interface(map['opcd_ctrl'], 'powerman')
      bus = SMBus(self.opcd.get('gpio_i2c_bus'))
      self.gpio_mosfet = GPIO_Bank(bus, self.opcd.get('gpio_i2c_address'))
      self.power_pin = self.opcd.get('gpio_power_pin')
      self.cells = self.opcd.get('battery_cells')
      self.low_cell_voltage = self.opcd.get('battery_low_cell_voltage')
      self.capacity = self.opcd.get('battery_capacity')
      self.low_battery_voltage = self.cells * self.low_cell_voltage
      self.critical = False

      # start threads:
      self.standing = True
      self.adc_thread = Thread(target=self.adc_reader)
      self.adc_thread.start()
      self.emitter_thread = Thread(target=self.power_state_emitter)
      self.emitter_thread.start()
      self.request_thread = Thread(target=self.request_handler)
      self.request_thread.start()
      self.warning_thread = Thread(target=self.battery_warning)


   def battery_warning(self):
      # do something in order to indicate a low battery:
      while True:
         for i in range(1, 8):
            self.gpio_mosfet.set_gpio(i, False)
            sleep(0.1)
         for i in range(1, 8):
            self.gpio_mosfet.set_gpio(i, True)
            sleep(0.1)


   def adc_reader(self):
      voltage_adc = ADC(self.opcd.get('voltage_adc'))
      current_adc = ADC(self.opcd.get('current_adc'))
      self.current_integral = 0.0
      while True:
         self.voltage = (voltage_adc.read() - 56.0) / 134.0
         self.current = current_adc.read() / 1024
         self.current_integral += self.current
         if self.voltage <= self.low_battery_voltage:
            try:
               self.warning_thread.start()
            except:
               pass
         sleep(1)


   def power_state_emitter(self):
      while True:
         state = PowerState()
         sleep(5)
         try:
            state.voltage = self.voltage
            state.current = self.current
            state.capacity = self.capacity
            state.consumed = self.current_integral
            state.remaining = self.capacity - self.current_integral
            if self.voltage <= self.low_battery_voltage:
               self.critical = True
            state.critical = self.critical
            state.estimate = state.remaining / self.current
         except AttributeError:
            continue
         except: # division by zero in last try block line
            pass
         self.monitor_socket.send(state.SerializeToString())


   def power_off(self):
      self.gpio_mosfet.set_gpio(self.power_pin, False)


   def request_handler(self):
      timeout = self.opcd.get('power_save_timeout')
      req = PowerRequest()
      rep = PowerReply()
      timer = None
      while True:
         rep.status = OK
         try:
            req_data = self.ctrl_socket.recv()
         except:
            sleep(1)
            continue
         try:
            req.ParseFromString(req_data)
         except:
            rep.status = E_SYNTAX
         else:
            try:
               timer.cancel()
            except:
               pass
            if req.command == STANDING:
               timer = Timer(timeout, self.power_off)
               timer.start()
            else:
               if self.critical:
                  # flying command is not allowed if battery was critical:
                  rep.status = E_POWER
               else:
                  self.gpio_mosfet.set_gpio(self.power_pin, True)
         self.ctrl_socket.send(rep.SerializeToString())



def main(name):
   PowerMan(name)


daemonize('powerman', main)

