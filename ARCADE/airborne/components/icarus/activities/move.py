
from math import hypot
from time import sleep
from util.geomath import lineq_n, meter_offset
from numpy import array, zeros
from core_pb2 import *
from activity import Activity, StabMixIn



class MoveActivity(Activity, StabMixIn):


   Z_MAX = 2.0


   def __init__(self, fsm, core, mon_data, msg):
      Activity.__init__(self)
      StabMixIn.__init__(self, core)
      self.fsm = fsm
      self.core = core
      self.mon_data = mon_data
      self.msg = msg
      self.canceled = False


   def run(self):
      # shortcut identifiers:
      move_data = self.msg.move_data
      mon_data = self.mon_data
      params = self.core.params

      # get xyz coordinates
      avail = [False] * 3
      xyz = zeros(3)
      if not self.msg.glob:
         # local coordinates: (x, y, z) = (p0, p1, p2)
         for i in xrange(3):
            name = 'p%d' % i
            if move_data.HasField(name):
               avail[i] = True
               xyz[i] = getattr(move_data, name)
      else:
         # global coordinates:
         start_pos = (params.start_lat, params.start_lon)
         xyz[0:2] = meter_offset(start_pos, xyz[0:2])
      xyz[2] = params.start_alt - xyz[2]

      if self.msg.rel:
         # if we move relative, we have to add the current
         # xyz coordinates to the new ones
         xyz += array([mon_data.x, mon_data.y, mon_data.z])

      if avail[0 : 2] == [False] * 2:
         # only z position is updated: set speed and position directly:
         if self.msg.HasField('speed'):
            self.core.set_ctrl_param(SPEED_Z, self.msg.speed)
         self.core.set_ctrl_param(POS_Z, xyz[2])
      else:
         # x, y are updated
         # set speed:
         if self.msg.HasField('speed'):
            self.core.set_ctrl_param(SPEED_XY, self.msg.speed)
         self.core.set_ctrl_param(SPEED_Z, self.Z_MAX)
         # set position
         self.core.set_ctrl_param(POS_X, xyz[0])
         self.core.set_ctrl_param(POS_Y, xyz[1])
         if move_data.HasField('p2'):
            # update z setpoint linearly between the starting position and destination:
            n = lineq_n(0.0, mon_data.z, move_data.p2)
            m = mon_z
            while not self.arrived:
               if self.cancel:
                  self.core.set_ctrl_param(POS_X, mon_data.x)
                  self.core.set_ctrl_param(POS_Y, mon_data.y)
                  self.stabilize()
                  return # not going into hovering state
               dist = euclid_dist(move_data.x)
               z = n * x + m
               self.core.set_ctrl_param(POS_Z, z)
               sleep(1)

      self.stabilize()
      self.fsm.done()


   def cancel(self):
      self.canceled = True

