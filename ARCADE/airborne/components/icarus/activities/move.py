
from math import hypot
from time import sleep
from util.geomath import lineq_n, meter_offset
from numpy import array, zeros
from core_pb2 import *
from activity import Activity, StabMixIn
from util.geomath import gps_add_meters
from util.srtm import SrtmElevMap


_srtm_elev_map = SrtmElevMap()


class MoveActivity(Activity, StabMixIn):


   Z_SPEED_MAX = 2
   SRTM_SAFETY_ALT = 20


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
 
      # kalman-filtered lat, lon:
      lat, lon = gps_add_meters(core.params.start_lat, core.params.start_lon,
                                self.setpoints[0], self.setpoints[1])

      coord = [None, None, None] # x, y, z setpoints
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
            
            if glob_sp[0] != None:
               lat += glob_sp[0]
            if glob_sp[1] != None:
               lon += glob_sp[1]
            coord[0], coord[1] = gps_meters_offset(core.params.start_lat,
                                                   core.params.start_lon,
                                                   lat, lon)
            if glob_sp[2] != None:
               coord[2] = self.icarus.setpoints[2] + glob_sp[2]
            else:
               coord[2] = self.icarus.setpoints[2]
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
      
      if arg.HasField('speed'):
         core.set_ctrl_param(SPEED_XY, arg.speed)
         #core.set_ctrl_param(SPEED_Z, self.Z_SPEED_MAX)
      
      # set position
      prev_z = self.icarus.setpoints[2]
      core.set_ctrl_param(POS_X, coord[0])
      core.set_ctrl_param(POS_Y, coord[1])
      self.icarus.setpoints = coord
      
      if coord[2] != prev_z:
         # update z setpoint linearly between the starting position and destination:
         n = lineq_n(0.0, prev_z, coord[2])
         if hypot(mon_data.x_err, mon_data.y_err) < self.LAT_STAB_EPSILON:
            if self.cancel:
               core.set_ctrl_param(POS_X, mon_data.x)
               core.set_ctrl_param(POS_Y, mon_data.y)
               self.stabilize()
               return # not going into hovering state
            dist = euclid_dist(move_data.x)
            z = n * x + m
            # check elevation map:
            srtm_z = _srtm_elev_map.lookup(lat, lon) - params.start_alt
            if z < srtm_alt + self.SRTM_SAFETY_ALT:
               z = srtm_alt + self.SRTM_SAFETY_ALT
            core.set_ctrl_param(POS_Z, z)
            sleep(1)
      
      self.stabilize()
      if not self.canceled:
         fsm.done()


   def _cancel(self):
      self.canceled = True

