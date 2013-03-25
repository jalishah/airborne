import sys
import zmq
import msgpack

#  Socket to talk to server
context = zmq.Context()
socket = context.socket(zmq.SUB)

print "Collecting updates from node 1"
socket.connect ("ipc:///tmp/scl_70011")

socket.setsockopt(zmq.SUBSCRIBE, "")

unp = msgpack.Unpacker()

# Process 5 updates
while True:
    string = socket.recv()
    unp.feed(string)
    for msg in unp:
	if type(msg) is tuple:
		print msg
		
