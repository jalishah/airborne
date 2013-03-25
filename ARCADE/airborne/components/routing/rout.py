import msgpack
import serial
import thread
import yaml
import zmq
import sys
from lookup import Lookup

# todo
# config.YAML 
# create a table for direct routes to send to neighbors.
# fix ground station id ?
# def rout_message(type , messsage , rcv_id)  to send the msg to send_recv component ... 

key_value = 0x00
my_id = 0x01

context = zmq.Context()
socket = context.socket(zmq.SUB)
# in dictionary (rout) the keys are the sender ids
rout = {};

print "Collecting updates from node 1"
socket.connect ("ipc:///tmp/scl_70012")
socket.setsockopt(zmq.SUBSCRIBE, "")

unp = msgpack.Unpacker()

# To check weather the key already exist in the routing table
def chk_rout_key( key ):
	if rout.has_key(key) == False:   # check if the key already exist
		set_new_route_key(key)
	return

# update rout add the value to routing table
def set_new_route_key( key ):
	global rout	
	rout[key] = []
	return	

#add value to the key if not exist already......

def add_new_rout_values( key, value):
	global rout
	look = Lookup(rout)
	links = look.get_value(key)

	if value not in links:
		rout[key].append(value)

	return


def get_rout (): 
	print "generate routing table: " , rout
	return

#check the type of message 
def chk_msgtype ( msg ):
	
	global key_value;
	key_value = my_id      # to add the self id or receiver id in the key part of rout (dict)
	chk_rout_key(key_value)   # this will be call once in the begining when it receive its first msg (msg of any type)
	add_new_rout_values(key_value, msg[0][1])   #add this sender to the key(this receiver)
	
	if msg[0][0] == 1:
		key_value = msg[0][1]  # to add the sender id in the value part of rout (dict)
		chk_rout_key(key_value)		# to add the sender id as a key if not exist
		for k in msg[1]:
			add_new_rout_values(key_value, k)  		
	return

while True:
	string = socket.recv()
	unp.feed(string)    # unpack the msg 
    	for msg in unp:
		if type(msg) is tuple:
		#	print msg
			chk_msgtype(msg)   # check the msg type  
	get_rout()

     


#if __name__ == "__main__":

