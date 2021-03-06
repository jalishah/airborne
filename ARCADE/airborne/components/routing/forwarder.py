import sys
import zmq
import msgpack
from storage import *


context = zmq.Context()

# subscribe to Mac layer to receive the message
sub_to_mac_socket = context.socket(zmq.SUB)
sub_to_mac_socket.connect ("ipc:///tmp/scl_70001")
sub_to_mac_socket.setsockopt(zmq.SUBSCRIBE, "")


#forward the application to mac_layer
pub_to_mac_socket = context.socket(zmq.PUB)
pub_to_mac_socket.bind("ipc:///tmp/scl_70002")



#forward the message to Application Layer
pub_to_app_socket = context.socket(zmq.PUB)
pub_to_app_socket.bind("ipc:///tmp/scl_70003")



#subscribe to application layer
sub_to_app_socket = context.socket(zmq.SUB)
sub_to_app_socket.connect (yaml_conf['command']['sub_to'])
sub_to_app_socket.connect (yaml_conf['chat']['sub_to'])
sub_to_app_socket.setsockopt(zmq.SUBSCRIBE, "")



def forward_msg(typ,self_id,org,ttl,msg_unq_id,recv_id,payload):
	
	prefix = [typ,self_id,org,ttl,msg_unq_id,recv_id]
	#print len(prefix)
	#print sys.getsizeof(prefix)
	my_message = msgpack.packb([prefix,payload])
	#print sys.getsizeof(my_message)
	#print len(my_message)
	pub_to_mac_socket.send(my_message)
