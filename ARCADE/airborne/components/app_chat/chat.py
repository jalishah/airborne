import zmq
import msgpack
from time import sleep


context = zmq.Context()
socket = context.socket(zmq.PUB)
socket.bind("ipc:///tmp/scl_70014")


sub_socket = context.socket(zmq.SUB)
sub_socket.connect ("ipc:///tmp/scl_70016")
sub_socket.setsockopt(zmq.SUBSCRIBE, "")


mtype = 0;
mreceiver = 0;

def send_msg():
	while 1:
		payload = raw_input("enter your short message") 
		my_message = msgpack.packb([mtype,mreceiver,payload])
		socket.send(my_message)
				
def receive_msg():
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
