
from opcd_interface import OPCD_Interface
from threading import Thread
import re


class ParamHandler(Thread):

   def __init__(self, dispatcher):
      Thread.__init__(self)
      self.dispatcher = dispatcher
      self.opcd_interface = OPCD_Interface('mavlink')
      self.param_map = {}
      self.param_rev_map = {}
      list = self.opcd_interface.get('')
      c = 0
      type_map = {float: MAV_VAR_FLOAT, long: MAV_VAR_INT32}
      cast_map = {float: float, long: int}
      for name, val in list:
         try:
            type = type_map[val.__class__]
            self.param_map[c] = name, type, cast_map[val.__class__]
            self.param_rev_map[c] = type, name, cast_map[val.__class__]
            c += 1
         except Exception, e:
            print str(e)
   
   def run(self):
      for e in self.dispatcher.generator('PARAM_'):
         print e
         if e.get_type() == 'PARAM_REQUEST_LIST':
            list = self.opcd_interface.get('')
            for index, (name, type, cast) in self.param_map.items():
               try:
                  val = self.opcd_interface.get(name)
                  #name_short = re.sub('(?P<foo>\w)\w*\.', '\g<foo>.', name)
                  name_short = re.sub('_', '-', name)
                  name_short = re.sub('\.', '_', name_short)
                  mavio.mav.param_value_send(name_short, float(val), type, len(self.param_map), index)
               except Exception, ex:
                  print str(ex)
         elif e.get_type() == 'PARAM_REQUEST_READ':
            index = e.param_index
            name, type, cast = self.param_map[index]
            try:
               val = self.opcd_interface.get(name)
               #name_short = re.sub('(?P<foo>\w)\w*\.', '\g<foo>.', name)
               name_short = re.sub('_', '-', name)
               name_short = re.sub('\.', '_', name_short)
               mavio.mav.param_value_send(name_short, float(val), type, len(self.param_map), index)
            except Exception, ex:
               print str(ex)


class DeadbeefHandler(Thread):

   def __init__(self, dispatcher):
      Thread.__init__(self)
      self.dispatcher = dispatcher

   def run(self):
      for e in self.dispatcher.generator('BAD_DATA'):
         print 'bad data ignored'

