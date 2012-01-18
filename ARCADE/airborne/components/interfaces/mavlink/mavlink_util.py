class ControlSensorBits:

   def __init__(self):
      self.cs_list = [
         'GYRO_3D',
         'ACC_3D',
         'MAG_3D',
         'PRESSURE_ABS',
         'PRESSURE_DIFF',
         'GPS',
         'OPTICAL_FLOW',
         'COMPUTER_VISION',
         'LASER_SCANNER',
         'VICON_LEICA',
         'ANGLE_RATE_CONTROL',
         'ATTITUDE_CTRL',
         'YAW_CTRL',
         'ALTITUDE_CTRL',
         'XY_CTRL',
         'MOTOR_CTRL'
      ]

   def bits(self, flag_names):
      bits = 0
      for i in range(0, len(self.cs_list)):
         if self.cs_list[i] in flag_names:
            bits |= 1 << i
      return bits


