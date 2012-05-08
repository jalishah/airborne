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


from time import sleep
from threading import Thread, Timer
from signal import pause
from smbus import SMBus
from os import system, sep
from logging import basicConfig as log_config, debug as log_debug
from logging import info as log_info, warning as log_warn, error as log_err
from logging import DEBUG

from mtputil import *
from power_pb2 import *
from scl import generate_map
from named_daemon import daemonize
from opcd_interface import OPCD_Interface
from timeutil import Hysteresis

from paths import user_data_dir
from hardware import ADC, GPIO_Bank


class ICARUS:

   def __init__(self, sockets):
      self.flight_time = 0

class PowerMan:

   def __init__(self, name):
      # set-up logger:
      logfile = user_data_dir() + sep + 'PowerMan.log'
      log_config(filename = logfile, level = DEBUG,
                 format = '%(asctime)s - %(levelname)s: %(message)s')
      # initialized and load config:
      log_info('powerman starting up')
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
      self.gpio_mosfet.write()
      self.warning_started = False

      # start threads:
      self.standing = True
      self.adc_thread = start_daemon_thread(self.adc_reader)
      self.emitter_thread = start_daemon_thread(self.power_state_emitter)
      self.request_thread = start_daemon_thread(self.request_handler)
      log_info('powerman running')


   def battery_warning(self):
      # do something in order to indicate a low battery:
      msg = 'CRITICAL WARNING: SYSTEM BATTERY VOLTAGE IS LOW; IMMEDIATE SHUTDOWN REQUIRED OR SYSTEM WILL BE DAMAGED'
      log_warn(msg)
      system('echo "%s" | wall' % msg)
      while True:
         self.gpio_mosfet.set_gpio(5, False)
         sleep(0.1)
         self.gpio_mosfet.set_gpio(5, True)
         sleep(0.1)


   def adc_reader(self):
      voltage_adc = ADC(self.opcd.get('voltage_adc'))
      current_adc = ADC(self.opcd.get('current_adc'))
      voltage_lambda = eval(self.opcd.get('adc_2_voltage'))
      current_lambda = eval(self.opcd.get('adc_2_current'))
      self.current_integral = 0.0
      hysteresis = Hysteresis(self.opcd.get('low_voltage_hysteresis'))
      while True:
         sleep(1)
         try:
            self.voltage = voltage_lambda(voltage_adc.read())  
            self.current = current_lambda(current_adc.read())
            self.current_integral += self.current / 3600
            print self.voltage, self.low_battery_voltage
            if self.voltage < self.low_battery_voltage:
               self.critical = hysteresis.set()
            else:
               hysteresis.reset()
            if self.critical:
               if not self.warning_started:
                  self.warning_started = True
                  start_daemon_thread(self.battery_warning)
         except Exception, e:
            log_err(str(e))

   def power_state_emitter(self):
      while True:
         state = PowerState()
         sleep(5)
         try:
            state.voltage = self.voltage
            state.current = self.current
            state.capacity = self.capacity
            state.consumed = self.current_integral
            remaining = self.capacity - self.current_integral
            if remaining < 0:
               remaining = 0
            state.remaining = remaining
            state.critical = self.critical
            state.estimate = state.remaining / self.current * 3600
            log_info(str(state).replace('\n', ' '))
         except AttributeError:
            continue
         except: # division by zero in last try block line
            pass
         self.monitor_socket.send(state.SerializeToString())


   def power_off(self):
      self.gpio_mosfet.set_gpio(self.power_pin, False)


   def request_handler(self):
      timeout = self.opcd.get('power_save_timeout')
      req = PowerReq()
      rep = PowerRep()
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
            if req.cmd == STAND_POWER:
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
   await_signal()


daemonize('powerman', main)

