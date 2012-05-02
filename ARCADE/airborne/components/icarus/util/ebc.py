
class BaroGroundController:

   def __init__(self, baro_setpoint, ground_min, kp):
      self.baro_setpoint = baro_setpoint
      self.baro_shift = 0.0
      self.ground_min = ground_min
      self.kp = kp

   def calc(self, dt, ground_dist):
      error = ground_dist - self.ground_min
      self.baro_shift = self.baro_shift - self.kp * error * dt
      if self.baro_shift < 0:
         self.baro_shift = 0
      return self.baro_setpoint + self.baro_shift

 
ebc = BaroGroundController(6, 5, 0.5)
altitudes = [0,1,1,2,2,2,2,1,1,1,3,4,5,6,7,8,9,10,11,11,11,11,11,11,10,9,8,7,6,7,8,9,10,9,7,5,3,1,0,0,1,2,3,2,1]
baro = ebc.baro_setpoint
for alt in altitudes:
   ultra = baro - alt # simulated ultrasonic value
   baro = ebc.calc(1.0, ultra)
   print alt, baro

