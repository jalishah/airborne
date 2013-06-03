import zmq
from zmq.devices import monitored_queue
from time import sleep
import thread
import sys
import msgpack

unp = msgpack.Unpacker()


def send_chat():
	context = zmq.Context()
	pub_socket = context.socket(zmq.PUB)
	pub_socket.bind("ipc:///tmp/scl_70016")
	global mreceiver
	global mtype
	while 1:
		payload = raw_input("enter your short message:    ")
		typ = int(raw_input("enter message type:    "))
		pub_socket.send("%s %s" % (str(typ),payload) )
				

def receive():
	ctx = zmq.Context()
	sub_socket = ctx.socket(zmq.SUB)
	sub_socket.connect ("ipc:///tmp/scl_70014")
	sub_socket.connect ("ipc:///tmp/scl_70015")
	sub_socket.setsockopt(zmq.SUBSCRIBE,'')	

	global i
	while 1:
		string = sub_socket.recv_multipart()
		print string

			
try:
	thread.start_new_thread(receive, () )
	thread.start_new_thread(send_chat, () )

except:
	print "Error: unable to start thread"

while True:
	sleep(1)
