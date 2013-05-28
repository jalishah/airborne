import sys
import zmq
import msgpack
from lookup import Lookup
from storage import *
from forwarder import *


unp = msgpack.Unpacker()

def create_header(msg):
	unq_id = 0
	if msg[0] == 4:
		print "sending batman"
		forward_msg(msg[0],my_id,my_id,3,unq_id,msg[1],msg[2])
		unq_id +=1
	if msg[0] == 2:
		# chat app .....
		print " i am sending chat"
		recv_id = find_rout_key(msg[1])   #search for receiver in the routing table if not exit broadcast.
		forward_msg(msg[0],my_id,msg[1],2,2,recv_id,msg[2])
		
	"""
		add more condition for more apps and types
	"""
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
		


