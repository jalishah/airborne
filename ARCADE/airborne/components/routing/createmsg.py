from lookup import Lookup
from storage import *
from forwarder import *


def create_header(msg):

	my_id = yaml_conf['header']['my_id']	
	try:
		if msg[0] == 4:
			print "sending batman"
			ttl = yaml_conf['header']['ttl_batman']
			seq_id = yaml_conf['header']['seq_id_batman']
			forward_msg(msg[0],my_id,my_id,ttl,seq_id,msg[1],msg[2])
		
		if msg[0] == 2:
			# chat app .....
			print " i am sending chat"
			ttl = yaml_conf['header']['ttl_chat']
			seq_id = yaml_conf['header']['seq_id_chat']	
			recv_id = find_rout_key(msg[1])   #search for receiver in the routing table if not exit broadcast.
			forward_msg(msg[0],my_id,msg[1],ttl,seq_id,recv_id,msg[2])
		
		if msg[0] == 1:
			# chat app .....
			print " i am sending chat"
			ttl = yaml_conf['header']['ttl_command']
			seq_id = yaml_conf['header']['seq_id_command']	
			recv_id = find_rout_key(msg[1])   #search for receiver in the routing table if not exit broadcast.
			forward_msg(msg[0],my_id,msg[1],ttl,seq_id,recv_id,msg[2])
		"""
			add more condition for more apps and types
		"""
	except TypeError:
		print "garbage value"
# Process 5 updates




