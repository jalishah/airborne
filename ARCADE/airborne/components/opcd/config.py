
#
# file: config.py
# purpose: overridable configuration fil based on yaml
#


import yaml
import copy
import os




class ConfigError(Exception):

   def __init__(self, msg):
      assert isinstance(msg, str)
      self.msg = msg

   def __str__(self):
      return self.msg




class Config:

   def __init__(self, config_prefix):
      self.LEAF_TYPES = [bool, int, float, str]
      assert isinstance(config_prefix, str)
      # build config paths from prefix:
      self.config_prefix = config_prefix
      self.base_path = config_prefix + '-base.yaml'
      self.overlay_path = config_prefix + '-overlay.yaml'
      # load base config and overlay of present:
      self.base = yaml.load(file(self.base_path))
      if os.path.isfile(self.overlay_path):
         self.overlay = yaml.load(file(self.overlay_path))
      else:
         self.overlay = {}
      # check single document integrity:
      for doc in self.base, self.overlay:
         self._check_tree(doc)
      # check inter-document integrity:
      for key in self._get_all_keys(self.overlay):
         overlay_class = self._find_entry(self.overlay, key).__class__
         try:
            base_class = self._find_entry(self.base, key).__class__
         except:
            raise ConfigError('overlay defines key "' + key + '", which does not exist in base')
         if overlay_class != base_class:
            raise ConfigError('different data types for overlay (' + str(overlay_class) + ') and base (' + str(base_class) + ') for key: ' + key)


   def set(self, key, val):
      '''
      set attribute identified by key to val
      '''
      if self._find_entry_or_none(self.base, key) == None:
         raise ConfigError('cannot override unknown attribute "' + key + '"')
      self._insert_val(self.overlay, key, val)


   def get(self, key):
      '''
      get attribute using key
      '''
      try:
         return self._find_entry(self.overlay, key)
      except KeyError:
         try:
            return self._find_entry(self.base, key)
         except KeyError:
            raise ConfigError(key + ' was not found in base config')


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
         dump = '#\n# generated file! DO NOT EDIT\n#\n\n'
         dump += yaml.dump(self.overlay, default_flow_style = False)
         overlay_file = file(self.overlay_path, 'w')
         overlay_file.write(dump)
         overlay_file.close()


   def _check_tree(self, node):
      if isinstance(node, dict):
         for key, node in node.iteritems():
            if isinstance(key, str) and '.' in key:
               raise ConfigError('key ' + str(key) + ' must not contain a dot (.) character')
            self._check_tree(node)
      else:
         if node.__class__ not in self.LEAF_TYPES:
            raise ConfigError('node ' + str(node) + ' must be one of: ' + str(self.LEAF_TYPES))


   def _insert_val(self, node, key, val):
      if isinstance(node, dict):
         head, tail = self._split_key(key)
         if not tail:
            node[head] = val
         else:
            if head not in node.keys():
               node[head] = {}
            node = node[head]
            self._insert_val(node, tail, val)


   def _get_all_keys(self, node):
      if isinstance(node, dict):
         list = []
         for key, node in node.iteritems():
            if not isinstance(node, dict):
               list.append(key)
            else:
               sub_list = self._get_all_keys(node)
               list.extend(map(lambda x : key + '.' + x, sub_list))
         return list


   def _find_entry_or_none(self, node, key):
      try:
         return self._find_entry(node, key)
      except KeyError:
         return


   def _split_key(self, key):
      if '.' in key:
         pos = key.find('.')
         head = key[0 : pos]
         tail = key[pos + 1 : ]
      else:
         head = key
         tail = None
      return head, tail

   
   def _delete_key(self, node, key):
      if isinstance(node, dict):
         head, tail = self._split_key(key)
         next_node = node[head]
         return self._delete_key(next_node, tail) + [(node, head)]
      else:
         return []


   def _clean_overlay(self):
      keys = self._get_all_keys(self.overlay)
      for key in keys:
         overlay_val = self._find_entry(self.overlay, key)
         base_val = self._find_entry(self.base, key)
         if overlay_val == base_val:
            l = self._delete_key(self. overlay, key)
            i = l[0]
            del i[0][i[1]]
            prev_i = i[0]
            for i in l[1:]:
               if len(prev_i) == 0:
                  del i[0][i[1]]
               else:
                  break
               prev_i = i[0]


   def _find_entry(self, node, key):
      if isinstance(node, dict):
         try:
            head, tail = self._split_key(key)
         except TypeError:
            return node
         node = node[head]
         return self._find_entry(node, tail)
      else:
         assert node.__class__ in self.LEAF_TYPES
         return node




if __name__ == '__main__':
   try:
      conf = Config('config')
      conf._clean_overlay()
      print conf.overlay
   except ConfigError, e:
      print e

