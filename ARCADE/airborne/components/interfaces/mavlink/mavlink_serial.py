
from pymavlink.mavlinkv10 import *
from pymavlink.mavutil import mavserial

from mavlink_source import MAVLinkSource
from gendisp import GenDisp


mav_serial = mavserial('/dev/ttyO3', 9600)

source = MAVSource(mav_serial)
dispatcher = GenDisp(source, True)
dispatcher.start()

for e in dispatcher.generator(''):
   print str(e)

