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
socket.bind("ipc:///tmp/scl_70012")
i = Interface('/dev/ttyACM1')

mcounter = 0

def send_msg():
	global mcounter	
	global i
	prefix = [0x01,0x01,0,0,0,0]
	payload = 'abcd'
		
	while 1:
		myMessage = msgpack.packb([prefix,payload])
		i.send(myMessage)
		mcounter += 1
		sleep(0.1)
	
def receive_msg():
	unp = msgpack.Unpacker()
	global i
	while 1:
		data = i.receive()
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
