import msgpack
import serial
import thread
import yaml
import zmq
import sys

# todo
# config.YAML 
# create a table for direct routes to send to neighbors.
# fix ground station id ?
# def rout_message(type , messsage , rcv_id)  to send the msg to send_recv component ... 

rout_value = 0x00

context = zmq.Context()
socket = context.socket(zmq.SUB)
# in dictionary (rout) the keys are the sender ids
rout = {};

print "Collecting updates from node 1"
socket.connect ("ipc:///tmp/scl_70012")
socket.setsockopt(zmq.SUBSCRIBE, "")

unp = msgpack.Unpacker()

# To check weather the key already exist in the routing table
def chk_rout( key ):
	if rout.has_key(key) == False:   # check if the key already exist
		set_route(key)
	return

# update rout add the value to routing table
def set_route( key ):
	global rout	
	rout[key] = rout_value
	return	


def get_rout (): 
	print "generate routing table: " , rout
	return

#check the type of message 
def chk_msgtype ( msg ):
	global rout_value;
	rout_value = 0x00      # to add the self id or receiver id in the value part of rout (dict)
	chk_rout(msg[0][1])
	
	if msg[0][0] == 1:
		rout_value = msg[0][1]  # to add the sender id in the value part of rout (dict)
		for k in msg[1]:
			chk_rout(k)		
	return

while True:
	string = socket.recv()
	unp.feed(string)    # unpack the msg 
    	for msg in unp:
		if type(msg) is tuple:
		#	print msg
			chk_msgtype(msg)   # check the msg type  
	get_rout()

     

#def main():
#	pass

#if __name__ == "__main__":
#   main()
