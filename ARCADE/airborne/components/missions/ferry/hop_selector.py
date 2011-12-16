
import random
from tsp import run_tsp
from histogram import Histogram
import math


class HopSelector:

   def register(self, ferry):
      self.ferry = ferry
      if hasattr(self, 'init'):
         self.init()


class RandomHopSelector(HopSelector):

   def get_next_hop(self):
      return random.choice(self.ferry.term_map.items())


class TSP_HopSelector(HopSelector):

   def init(self):
      self.order_index = 0 # used to iterate through TSP generated order
      self.order = run_tsp(self.ferry.term_map.values())
      print self.order

   def get_next_hop(self):
      self.order_index = (self.order_index + 1) % len(self.order)
      return self.ferry.term_map.items()[self.order[self.order_index]]


class HistogramHopSelector(HopSelector):

   def init(self):
      self.current_term = 0

   def dist(self, a, b):
      return math.sqrt((a[0] - b[0]) ** 2 + (a[1] - b[1]) ** 2)

   def term_pos(self, term):
      return self.ferry.term_map[term]

   def get_next_hop(self):
      hist = Histogram()
      print hist
      for message in self.ferry.messages:
         hist.count(message.dest)
      priorities = hist.norm_hist()
      hop = random.choice(self.ferry.term_map.items())
      max_rating = 0.0
      for term in self.ferry.term_map.keys():
         try:
            prio = priorities[term]
            d = self.dist(self.term_pos(self.current_term), self.term_pos(term))
            rating = priorities[term] / self.dist(self.term_pos(self.current_term), self.term_pos(term))
         except:
            rating = 0.0
         if rating > max_rating:
            max_rating = rating
            hop = term, self.term_pos(term)
      self.current_term = hop[0]
      return hop

