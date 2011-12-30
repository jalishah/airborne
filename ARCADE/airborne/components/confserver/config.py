
#
# file: config.py
# purpose: overridable configuration fil based on yaml
#


import yaml
import copy
import os


class ConfigError(Exception):

   def __init__(self, msg):
      self.msg = msg

   def __repr__(self):
      return repr(self.msg)


class Config:

   def __init__(self, config_prefix):
      assert isinstance(config_prefix, str)
      # build config paths from prefix:
      self.config_prefix = config_prefix
      self.base_path = config_prefix + '-base.yml'
      self.overlay_path = config_prefix + '-overlay.yml'
      # load base config and overlay of present:
      self.base = yaml.load(file(self.base_path))
      if os.path.isfile(self.overlay_path):
         self.overlay = yaml.load(file(self.overlay_path))
      else:
         self.overlay = {}
      # check single document integrity:
      for doc in self.base, self.overlay:
         self._check(doc)
      # check inter-document integrity:
      for section, entry in self.overlay.items():
         for key, val in entry.items():
            assert val.__class__ == self.overlay[section][key].__class__


   def _check(self, doc):
      assert isinstance(doc, dict)   
      for section, entry in doc.items():
         assert isinstance(section, str)
         assert isinstance(entry, dict)
         for key, val in entry.items():
            assert isinstance(key, str)
            assert val.__class__ in [str, int, float]


   def set(self, section, key, val):
      '''
      set attribute [section, key] to val
      '''
      if section not in self.base or key not in self.base[section]:
         raise ValueError('base attribute not known to config file: ' + section + '.' + key)
      # add attribut to overlay:
      if section not in self.overlay:
         self.overlay[section] = {}
      else:
         assert val.__class__ == self.overlay[section][key].__class__
      self.overlay[section][key] = val
      # remove attribute from overlay if equal:
      if self.base[section][key] == self.overlay[section][key]:
         del(self.overlay[section][key])
         if len(self.overlay[section]) == 0:
            del self.overlay[section]


   def get(self, section, key):
      '''
      get attribute using section, key
      '''
      try:
         return self.overlay[section][key]
      except KeyError:
         try:
            return self.base[section][key]
         except KeyError:
            raise ConfigError(section + '.' + key + ' was undefined in base config')


   def persist(self): 
      '''
      write configuration overlay to filesystem
      '''
      if len(self.overlay) == 0:
         try:
            os.unlink(self.overlay_path)
         except:
            pass
      else:
         dump = yaml.dump(self.overlay, default_flow_style = False)
         overlay_file = file(self.overlay_path, 'w')
         overlay_file.write(dump)
         overlay_file.close()


if __name__ == '__main__':
   conf = Config('config')
   conf.set('kalman', 'process', 1.0e-1)
   conf.get('kalman', 'process')
   conf.persist()

