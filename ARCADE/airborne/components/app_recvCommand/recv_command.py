import zmq
from time import sleep
import thread
import sys
import msgpack
from subprocess import call

context = zmq.Context()
pub_socket = context.socket(zmq.PUB)
pub_socket.bind("ipc:///tmp/scl_70014")


sub_socket = context.socket(zmq.SUB)
sub_socket.connect ("ipc:///tmp/scl_70016")
sub_socket.setsockopt(zmq.SUBSCRIBE, "1")


mtype = 0;
mreceiver = 0;

def send_command():
	global mreceiver
	global mtype
	while 1:
		payload = raw_input("enter Command:    ")
		recv_id = raw_input("enter receiver id:     ")
		mreceiver = int(recv_id)		
		typ_id = raw_input("enter message type:     ")
		mtype =  int(typ_id) 		
		my_message = msgpack.packb([mtype,mreceiver,payload])
		pub_socket.send(my_message)
				
def receive_command():
	global i
	while 1:
		string = sub_socket.recv()
		print string
		sp_string = string.split( );

# list of commands to be executed are provided here 

		if sp_string[1] == 'ls'	:
			call(["ls", "-l"])
	
		if sp_string[1] == 'pwd':
			call(["pwd"])

try:
	thread.start_new_thread(receive_command, () )
	thread.start_new_thread(send_command, () )

except:
	print "Error: unable to start thread"

while True:
	sleep(1)
