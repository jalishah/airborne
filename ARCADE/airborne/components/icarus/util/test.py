from geomath import gps_proj_xy

dx = 0.0 * 1000
dy = 1000.0 * 1000

print gps_proj_xy((50.0, 10.0), (dx, dy))
