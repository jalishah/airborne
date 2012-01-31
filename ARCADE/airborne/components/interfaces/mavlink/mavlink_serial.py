
from pymavlink.mavlinkv10 import *
from pymavlink.mavutil import mavserial

from mavlink_source import MAVLinkSource
from gendisp import GenDisp
from opcd_interface import OPCD_Interface


opcd_interface = OPCD_Interface('mavlink')
mav_serial = mavserial('/dev/ttyO3', 9600)


#opcd_interface.get(key)
#opcd_interface.set(key, val)
#opcd_interface.persist()

source = MAVSource(mav_serial)
dispatcher = GenDisp(source, True)
dispatcher.start()

for e in dispatcher.generator(''):
   print str(e)
   if 

