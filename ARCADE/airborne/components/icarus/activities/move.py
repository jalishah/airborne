
from math import hypot
from time import sleep
from util.geomath import lineq_n, meter_offset
from numpy import array, zeros
from core_pb2 import *
from activity import Activity, StabMixIn



class MoveActivity(Activity, StabMixIn):


   Z_MAX = 2.0


   def __init__(self, icarus):
      Activity.__init__(self, icarus)


   def run(self):
      # shortcut identifiers:
      arg = self.icarus.arg
      move_data = arg.move_data
      mon_data = self.icarus.mon_data
      core = self.icarus.core
      params = core.params
      fsm = self.icarus.fsm

      # update x,y,z in setpoints:
      coord = [None, None, None]
      if arg.glob:
         # set global lat, lon postion:
         glob_sp = [None, None, None]
         for i in xrange(3):
            name = 'p%d' % i
            if move_data.HasField(name):
               glob_sp[i] = getattr(move_data, name)
         
         if arg.rel:
             # interpret lat, lon, alt as relative
             # covert x and y setpoints to rad, using start_lat, start_lon
             for i in xrange(3):
               name = 'p%d' % i
               if move_data.HasField(name):
                  coord[i] = self.icarus.setpoints[i] + getattr(move_data, name)
         else:
            # interpret lat, lon, alt as absolute
            start_pos = (params.start_lat, params.start_lon)
            if glob_sp[0] != None and glob_sp[1] != None:
               coord[0 : 2] = meter_offset(start_pos, glob_sp[0 : 2])
            elif glob_sp[0] != None:
               glob_sp[1] = 0.0
               coord[0] = meter_offset(start_pos, glob_sp[0 : 2])[0]
            elif glob_sp[1] != None:
               glob_sp[0] = 0.0
               coord[1] = meter_offset(start_pos, glob_sp[0 : 2])[1]
            # set global altitude:
            if glob_sp[2] != None:
               coord[2] = glob_sp[2] - params.start_alt
            else:
               coord[2] = self.icarus.setpoints[2]
      else:
         # local position update:
         for i in xrange(3):
            name = 'p%d' % i
            if move_data.HasField(name):
               if arg.rel:
                  # relative local coordinate:
                  coord[i] = self.icarus.setpoints[i] + getattr(move_data, name)
               else:
                  # absolute local coordinate:
                  coord[i] = getattr(move_data, name)
            else:
               coord[i] = self.icarus.setpoints[i]
      print coord
      self.icarus.setpoints = coord
      assert None not in coord
      fsm.done()
      return
      
      if self.msg.glob:
         # adapt coord to global coordinates:
         start_pos = (params.start_lat, params.start_lon)
         coord[0:2] = meter_offset(start_pos, coord[0:2])

      if self.msg.rel:
         # if we move relative, we have to add the current
         # coord coordinates to the new ones
         coord += array([mon_data.x, mon_data.y, mon_data.z])

      if avail[0 : 2] == [False] * 2:
         # only z position is updated: set speed and position directly:
         if self.msg.HasField('speed'):
            core.set_ctrl_param(SPEED_Z, self.msg.speed)
         core.set_ctrl_param(POS_Z, coord[2])
      else:
         # x, y are updated
         # set speed:
         if self.msg.HasField('speed'):
            core.set_ctrl_param(SPEED_XY, self.msg.speed)
         core.set_ctrl_param(SPEED_Z, self.Z_MAX)
         # set position
         core.set_ctrl_param(POS_X, coord[0])
         core.set_ctrl_param(POS_Y, coord[1])
         if move_data.HasField('p2'):
            # update z setpoint linearly between the starting position and destination:
            n = lineq_n(0.0, mon_data.z, move_data.p2)
            m = mon_z
            while not self.arrived:
               if self.cancel:
                  core.set_ctrl_param(POS_X, mon_data.x)
                  core.set_ctrl_param(POS_Y, mon_data.y)
                  self.stabilize()
                  return # not going into hovering state
               dist = euclid_dist(move_data.x)
               z = n * x + m
               core.set_ctrl_param(POS_Z, z)
               sleep(1)
      
      self.stabilize()
      if not self.canceled:
         fsm.done()


   def _cancel(self):
      self.canceled = True

