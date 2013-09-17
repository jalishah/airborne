import zmq
from time import sleep
import thread
import sys
import msgpack

context = zmq.Context()
pub_socket = context.socket(zmq.PUB)
pub_socket.bind("ipc:///tmp/scl_70015")



sub_socket = context.socket(zmq.SUB)
sub_socket.connect ("ipc:///tmp/scl_70003")
sub_socket.setsockopt(zmq.SUBSCRIBE, "2")

mtype = 'chat';
				
mreceiver = 2;
global i
while 1:
	string = sub_socket.recv()
	payload = "ACKlkjhgfdsaqwertzuiopn"
	my_message = msgpack.packb([mtype,mreceiver,payload])
	pub_socket.send(my_message)


