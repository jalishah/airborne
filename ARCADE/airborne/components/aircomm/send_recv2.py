import msgpack
from interface import Interface
from time import sleep
import serial
import thread
import yaml
import zmq
from random import randint


context = zmq.Context()
socket = context.socket(zmq.PUB)
socket.bind("ipc:///tmp/scl_70012")
i = Interface('/dev/ttyACM1')

mcounter = 0

def send_msg():
	global mcounter	
	global i
	#typ, sender, originator incase of batman, ttl , unique msg id , recevier incase of cmd msg
	payload = ''
		
	while 1:
		prefix = [4,2,2,3,randint(2,7),0]    
		myMessage = msgpack.packb([prefix,0])
		i.send(myMessage)
		mcounter += 1
		sleep(2)
	
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
