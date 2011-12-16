
from scl import generate_map
from dispatcher import Dispatcher
from protocols.arbiter_driver import ArbiterProtocolDriver
from protocols.core_interface import CoreInterface
from protocols.state_update_interface import StateUpdateInterface


sockets= generate_map('arbiter')
core = CoreInterface(sockets['core'])
sui = StateUpdateInterface(sockets['hlsm'])
disp = Dispatcher(core, sui)
apd = ArbiterProtocolDriver(sockets['ctrl'], disp)
apd.run()

