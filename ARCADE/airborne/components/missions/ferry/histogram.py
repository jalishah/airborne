
class Histogram:

   def __init__(self):
      self.hist = {}

   def count(self, bin):
      try:
         self.hist[bin] += 1
      except:
         self.hist[bin] = 1

   def norm_hist(self):
        max_val = 0
        for val in self.hist.values():
           if val > max_val:
              max_val = val
        if max_val == 0:
           return None

        norm_hist = {}
        for key in self.hist.keys():
           norm_hist[key] = float(self.hist[key]) / max_val
        return norm_hist

