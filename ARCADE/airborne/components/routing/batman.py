import msgpack
import serial
import thread
import yaml
import zmq
import sys
from lookup import Lookup
from createmsg import create_msg
from createmsg import forward_batman
from time import sleep

# todo
# config.YAML 
# create a table for direct routes to send to neighbors.
# fix ground station id ?
# def rout_message(type , messsage , rcv_id)  to send the msg to send_recv component ... 

key_value = 0x00
my_id = 0x01

context = zmq.Context()
# subscribe to Mac layer to receive the message
sub_socket = context.socket(zmq.SUB)
sub_socket.connect ("ipc:///tmp/scl_70011")
sub_socket.setsockopt(zmq.SUBSCRIBE, "")

#forward the message to Application Layer
pub_socket = context.socket(zmq.PUB)
pub_socket.bind("ipc:///tmp/scl_70016")



# in dictionary (rout) the keys are the sender ids
rout = {};

print "Collecting updates from nodes"

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
	
	if msg[0][0] == 4:
		key_value = msg[0][1]  # to add the sender id in the value part of rout (dict)
		chk_rout_key(key_value)		# to add the sender id as a key if not exist

		if key_value is not msg [0][2]:		# to avoid connecting same id's incase of sender and org is same
			add_new_rout_values(key_value,msg[0][2])
		
		if msg[0][3] > 0:
			new_ttl = msg[0][3] - 1
			originator = msg[0][2]
			unq_id = msg[0][4]
			fwdbatman = [new_ttl,originator]
			forward_batman(msg[0][0],originator,new_ttl,unq_id)

	if msg[0][0] == 3:
		pub_socket.send(msg[1])  		
	return

def sub_to_mac():

	while 1:
		string = sub_socket.recv()
		unp.feed(string)    # unpack the msg 
	    	for msg in unp:
			if type(msg) is tuple:
			#	print msg
				chk_msgtype(msg)   # check the msg type  
		get_rout()

     
try:
	thread.start_new_thread(sub_to_mac, () )
	thread.start_new_thread(create_msg, () )

except:
	print "Error: unable to start thread"

while True:
	sleep(1)


#if __name__ == "__main__":

