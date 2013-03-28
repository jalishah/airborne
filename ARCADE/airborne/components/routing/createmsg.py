import sys
import zmq
import msgpack
from lookup import Lookup

#  Socket to talk to server
context = zmq.Context()

print "Collecting msg from app_heart_beat"
sub_socket = context.socket(zmq.SUB)
sub_socket.connect ("ipc:///tmp/scl_70014")
sub_socket.setsockopt(zmq.SUBSCRIBE, "")

#forward the heartbeat to mac_layer
pub_socket = context.socket(zmq.PUB)
pub_socket.bind("ipc:///tmp/scl_70015")

unp = msgpack.Unpacker()

def create_msg(msg):
	if msg[0] == 0:
		# broadcast msg directly .....
		print " i am ok"
		my_message = msgpack.packb([msg[0],msg[1],msg[2]])
		pub_socket.send(my_message)
	if msg[0] == 1:
		# get the routing table entries directly connected to this node and broadcast
		print " i am not ok"

# Process 5 updates
while True:
    	string = sub_socket.recv()
	unp.feed(string)    # unpack the msg 
    	for msg in unp:
		if type(msg) is tuple:
			create_msg(msg)	
