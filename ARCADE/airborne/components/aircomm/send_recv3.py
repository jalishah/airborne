import msgpack
from interface import Interface
from time import sleep
import serial
import thread
import yaml
import zmq
import random

context = zmq.Context()
socket = context.socket(zmq.PUB)
socket.bind("ipc:///tmp/scl_70013")

i = Interface('/dev/ttyACM2')

mcounter = 0

def send_msg():
	global mcounter	
	global i	
	prefix = [0x00,0x00,0x00,0x00,0x00,0x00]
	payload = "antena3"			
	while 1:
		prefix[1] = random.randint(2,127)
		myMessage = msgpack.packb([prefix,payload])
		i.send(myMessage)
		mcounter += 1
		sleep(0.1)
	
def receive_msg():
	unp = msgpack.Unpacker()
	global i
	while 1:
		data = i.receive()
		#print len(data)		
		socket.send(data)
		unp.feed(data)
		for msg in unp:
			if type(msg) is tuple:
				print msg
					
try:
	thread.start_new_thread(receive_msg, () )
	thread.start_new_thread(send_msg, () )


except:
	print "Error: unable to start thread"

while True:
	sleep(1)
