import sys
import zmq
import msgpack
from lookup import Lookup
from storage import *
from forwarder import *


unp = msgpack.Unpacker()

def create_header(msg):
	if msg[0] == 0:
		# broadcast msg directly .....
		print " i am ok"
	#	my_message = msgpack.packb([msg[0],msg[1],msg[2]])
	#	pub_socket.send(my_message)
	if msg[0] == 3:
		# broadcast msg directly .....
		print " i am sending chat"
	#	my_message = msgpack.packb([msg[0],msg[1],msg[2]])
	#	pub_socket.send(my_message)
		
	if msg[0] == 1:
		# get the routing table entries directly connected to this node and broadcast
		print " i am not ok"

# Process 5 updates

def create_msg():
	global sub_to_app_socket
	while 1:
		print "Hello i m here to create ur msg"	
	    	string = sub_to_app_socket.recv()
		unp.feed(string)    # unpack the msg 
	    	for msg in unp:
			if type(msg) is tuple:
				create_header(msg)
		


