

class MissionList:

   def __init__(self):
      self._index = None
      self._items = []

   def append(self, item):
      self._items.append(item)

   def active_idx(self):
      '''
      return current active mission index or None
      '''
      return self._index

   def active(self):
      '''
      return current active mission item or None
      '''
      try:
         return self._items[self._index]
      except:
         pass

   def completed(self):
      '''
      go to the next mission item in the sequence
      '''
      self._index = (self._index + 1) % len(self._items)

   def set_active(self, index):
      '''
      set the current active mission item
      '''
      if index >= len(self._items):
         raise IndexError
      self._index = index

