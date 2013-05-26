import sys
import zmq
import msgpack
from storage import *


context = zmq.Context()
#forward the message to Application Layer
pub_to_app_socket = context.socket(zmq.PUB)
pub_to_app_socket.bind("ipc:///tmp/scl_70016")

#subscribe to application layer
print "Collecting msg from app_heart_beat"
sub_to_app_socket = context.socket(zmq.SUB)
sub_to_app_socket.connect ("ipc:///tmp/scl_70014")
sub_to_app_socket.setsockopt(zmq.SUBSCRIBE, "")


# subscribe to Mac layer to receive the message
sub_to_mac_socket = context.socket(zmq.SUB)
sub_to_mac_socket.connect ("ipc:///tmp/scl_70011")
sub_to_mac_socket.setsockopt(zmq.SUBSCRIBE, "")


#forward the application to mac_layer
pub_to_mac_socket = context.socket(zmq.PUB)
pub_to_mac_socket.bind("ipc:///tmp/scl_70015")



def forward_msg(typ,self_id,org,ttl,msg_unq_id,recv_id):
	prefix = [typ,self_id,org,ttl,msg_unq_id,recv_id]
	my_message = msgpack.packb([prefix,0])
	pub_to_mac_socket.send(my_message)

