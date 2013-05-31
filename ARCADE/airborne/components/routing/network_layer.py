import msgpack
import serial
import thread
import yaml
import zmq
import sys
from time import sleep
from msg_processor import chk_msgtype
from storage import *
from forwarder import *
from createmsg import create_header
from start_batman import start_batman

# config.YAML 

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
	global sub_to_app_socket
	while 1:
		string = sub_to_app_socket.recv()
		unp.feed(string)    # unpack the msg 
	    	for msg in unp:
			if type(msg) is tuple:
				create_header(msg)
		
try:
	thread.start_new_thread(sub_to_mac, () )
	thread.start_new_thread(sub_to_app, () )
	thread.start_new_thread(start_batman, ())
	thread.start_new_thread(empty_uniq_id_list,())

except:
	print "Error: unable to start thread"

while True:
	sleep(1)
