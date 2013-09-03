import msgpack
import yaml
import zmq
from forwarder import *
from msg_processor import chk_msgtype
from createmsg import create_header
from start_batman import start_batman

unp = msgpack.Unpacker()

def sub_to_mac():
	while 1:
		string = sub_to_mac_socket.recv()
		unp.feed(string)    # unpack the msg 
	    	for msg in unp:
			if type(msg) is tuple:
				print msg
				chk_msgtype(msg)   # check the msg type  
		get_rout()

def sub_to_app():
	while 1:
		string = sub_to_app_socket.recv()
		unp.feed(string)    # unpack the msg 
	    	for msg in unp:
			if type(msg) is tuple:
				create_header(msg)
