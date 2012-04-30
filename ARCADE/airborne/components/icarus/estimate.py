

class BatteryEstimate:


   WEIGHT = 1.2
   CAPACITY = 5000

   def dist_to_time(self, dist, speed):
      return dist / speed

   def time_to_fuel(self, weight, time):
      return 

   def dist_to_fuel(self, dist):
      return self.time_to_fuel(self.dist_to_time(dist))


