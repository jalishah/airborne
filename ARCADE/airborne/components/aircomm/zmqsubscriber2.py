import sys
import zmq

#  Socket to talk to server
context = zmq.Context()
socket = context.socket(zmq.SUB)

print "Collecting updates from node 2"
socket.connect ("ipc:///tmp/scl_70012")

socket.setsockopt(zmq.SUBSCRIBE, "")

# Process 5 updates
while True:
    string = socket.recv()
    print string
