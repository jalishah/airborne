import zmq
from time import sleep
import thread
import sys
import msgpack

context = zmq.Context()
pub_socket = context.socket(zmq.PUB)
pub_socket.bind("ipc:///tmp/scl_70016")



unp = msgpack.Unpacker()


def send_chat():
	global mreceiver
	global mtype
	while 1:
		payload = raw_input("enter your short message:    ")
		typ = int(raw_input("enter message type:    "))
		pub_socket.send("%s %s" % (str(typ),payload) )
				

def receive_chat():
	sub_socket = context.socket(zmq.SUB)
	sub_socket.connect ("ipc:///tmp/scl_70014")
	sub_socket.setsockopt(zmq.SUBSCRIBE,'')

	global i
	while 1:
		string = sub_socket.recv()
		print string
		#unp.feed(string)    # unpack the msg 
	    	#for msg in unp:
		#	print msg

def receive_command():
	sub_socket = context.socket(zmq.SUB)
	sub_socket.connect ("ipc:///tmp/scl_70015")
	sub_socket.setsockopt(zmq.SUBSCRIBE,'')

	global i
	while 1:
		string = sub_socket.recv()
		print string
		#unp.feed(string)    # unpack the msg 
	    	#for msg in unp:
		#	print msg
			
try:
	thread.start_new_thread(receive_chat, () )
	thread.start_new_thread(send_chat, () )
	thread.start_new_thread(receive_command, () )

except:
	print "Error: unable to start thread"

while True:
	sleep(1)
