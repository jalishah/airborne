
from scl import generate_map
from icarus_client import ICARUS_Client
from icarus_interface import ICARUS_Interface


c = ICARUS_Client(generate_map('cmdshell')['ctrl'])
i = ICARUS_Interface(c)
i.takeoff(z = 1, speed = 2)

