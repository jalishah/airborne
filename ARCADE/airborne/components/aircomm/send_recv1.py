from interface import Interface
from time import sleep
import serial
import thread
import yaml
import zmq
import random


context = zmq.Context()
pub_socket = context.socket(zmq.PUB)
pub_socket.bind("ipc:///tmp/scl_70001")


# get msg from network Layer
sub_socket = context.socket(zmq.SUB)
sub_socket.connect ("ipc:///tmp/scl_70002")
sub_socket.setsockopt(zmq.SUBSCRIBE, "")

i = Interface('/dev/ttyACM0')
mcounter = 0

def send_msg():
	while 1:
		string = sub_socket.recv()		
		#print "msg received"
		i.send(string)
		#	sleep(0.8)
	
def receive_msg():
	global i
	while 1:
		data = i.receive()
		pub_socket.send(data)	
		
try:
	thread.start_new_thread(receive_msg, () )
	thread.start_new_thread(send_msg, () )

except:
	print "Error: unable to start thread"

while True:
	sleep(1)
