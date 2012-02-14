
from mavutil import mavserial, mavudp
from scl import generate_map
from util.gendisp import GenDisp
from util.mavlink_util import MAVLink_Interface
from handlers.params import ParamHandler
from handlers.mission import MissionHandler
from handlers.deadbeef import DeadbeefHandler
from bridges.core_bridge import CoreBridge
from bridges.gps_bridge import GpsBridge
from bridges.arbiter_bridge import ArbiterBridge
from time import sleep


import pycallgraph
pycallgraph.start_trace()

# get socket map and open mavio:
socket_map = generate_map('mavlink')
simulate_local = False
if simulate_local:
   mavio = mavudp('localhost:14550', False, source_system = 0x01, blocking = True)
else:
   mavio = mavserial('/dev/ttyUSB1', 9600, source_system = 0x01)

# start parameter handlers and dispatchers:
dispatcher = GenDisp(mavio, True)
handlers = [ParamHandler(dispatcher),
   MissionHandler(dispatcher),
   DeadbeefHandler(dispatcher)]
dispatcher.start(handlers)

# start bridges:
mav_iface = MAVLink_Interface(mavio)
core_bridge = CoreBridge(socket_map, mav_iface, 1.0)
gps_bridge = GpsBridge(socket_map, mav_iface, 1.0)
arbiter_bridge = ArbiterBridge(socket_map, mav_iface, 1.0)

sleep(10)
pycallgraph.make_dot_graph('test.png')
