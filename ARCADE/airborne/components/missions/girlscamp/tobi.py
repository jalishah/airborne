
from random import choice, random


class Controller:
    
    def __init__(self):
        step = 0.5
        self.options = [-step, 0.0, step]
        self.hist = [(0.0, 0.0, -91.0)] * 10
        self.explore_prob = 0.8
        self.dec_hist = [(0.0, 0.0)] * 7
    
    def decide(self, x, y, s, timeStamp):
        self.hist = self.hist[1:] + [(x, y, s)]
        if random() < self.explore_prob:
            decision = choice(self.options), choice(self.options)
            x, y = decision[0] + x, decision[1] + y
        else:
            min = self.hist[0][2]
            x, y = self.hist[0][0], self.hist[0][1]
            for entry in self.hist[1:]:
               if min < entry[2]:
                   print min
                   min = entry[2]
                   x, y = entry[0], entry[1]
        return x, y, True

