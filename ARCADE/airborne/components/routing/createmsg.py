import sys
import zmq
import msgpack
from lookup import Lookup

#  Socket to talk to server
context = zmq.Context()
my_id = 0x01
print "Collecting msg from app_heart_beat"
sub_socket = context.socket(zmq.SUB)
sub_socket.connect ("ipc:///tmp/scl_70014")
sub_socket.setsockopt(zmq.SUBSCRIBE, "")

#forward the application to mac_layer
pub_socket = context.socket(zmq.PUB)
pub_socket.bind("ipc:///tmp/scl_70015")

unp = msgpack.Unpacker()

def create_header(msg):
	if msg[0] == 0:
		# broadcast msg directly .....
		print " i am ok"
		my_message = msgpack.packb([msg[0],msg[1],msg[2]])
		pub_socket.send(my_message)
	if msg[0] == 3:
		# broadcast msg directly .....
		print " i am sending chat"
		my_message = msgpack.packb([msg[0],msg[1],msg[2]])
		pub_socket.send(my_message)
		
	if msg[0] == 1:
		# get the routing table entries directly connected to this node and broadcast
		print " i am not ok"

# Process 5 updates

def create_msg():
	while 1:
		print "Hello i m here to create ur msg"	
	    	string = sub_socket.recv()
		unp.feed(string)    # unpack the msg 
	    	for msg in unp:
			if type(msg) is tuple:
				create_header(msg)
		

def forward_batman(typ,org,ttl,msg_unq_id):
	my_message = msgpack.packb([typ,my_id,org,ttl,msg_unq_id,0,0])
	pub_socket.send(my_message)

