
from scl import generate_map
from util.mavio import MAVIO_Serial
from util.gendisp import GenDisp
from util.mavlink_util import MAVLink_Interface
from handlers.params import ParamHandler
from handlers.mission import MissionHandler
from handlers.deadbeef import DeadbeefHandler
from bridges.core_bridge import CoreBridge
from bridges.gps_bridge import GpsBridge
from bridges.arbiter_bridge import ArbiterBridge
from time import sleep


# get socket map and open mavio:
socket_map = generate_map('mavlink')
mavio = MAVIO_Serial('/dev/ttyUSB1', 9600, source_system = 1)
arbiter_interface = ArbiterInterface(socket_map['arbiter_ctrl'])

# start parameter handlers and dispatcher (handles all incoming data):
dispatcher = GenDisp(mavio, True)
handlers = [ParamHandler(dispatcher),
   MissionHandler(dispatcher, arbiter_interface),
   DeadbeefHandler(dispatcher)]
dispatcher.start(handlers)

# start outgoing bridges:
mav_iface = MAVLink_Interface(mavio)
core_bridge = CoreBridge(socket_map, mav_iface, 1.0)
gps_bridge = GpsBridge(socket_map, mav_iface, 1.0)
arbiter_bridge = ArbiterBridge(socket_map, mav_iface, 1.0)

