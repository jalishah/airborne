import zmq
from time import sleep
import thread
import sys
import msgpack

context = zmq.Context()
pub_socket = context.socket(zmq.PUB)
pub_socket.bind("ipc:///tmp/scl_70014")


sub_socket = context.socket(zmq.SUB)
sub_socket.connect ("ipc:///tmp/scl_70016")
sub_socket.setsockopt(zmq.SUBSCRIBE, "")


mtype = 2;
mreceiver = 0x05;

def send_chat():
	while 1:
		payload = raw_input("enter your short message:    ") 
		my_message = msgpack.packb([mtype,mreceiver,payload])
		pub_socket.send(my_message)
				
def receive_chat():
	global i
	while 1:
		string = sub_socket.recv()
		print string

try:
	thread.start_new_thread(receive_chat, () )
	thread.start_new_thread(send_chat, () )

except:
	print "Error: unable to start thread"

while True:
	sleep(1)
