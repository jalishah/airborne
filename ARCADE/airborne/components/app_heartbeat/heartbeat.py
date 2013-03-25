import zmq
import msgpack
from time import sleep


context = zmq.Context()
socket = context.socket(zmq.PUB)
socket.bind("ipc:///tmp/scl_70014")

mtype = 0;
mreceiver = 0;

while 1:
	payload = "hello" 
	my_message = msgpack.packb([mtype,mreceiver,payload])
	socket.send(my_message)
	sleep(1.5)
