from lookup import Lookup
from storage import *
from forwarder import *


def create_header(msg):

	my_id = yaml_conf['header']['my_id']	
	try:
		if msg[0] == 'batman':
			print "sending batman"
			typ = yaml_conf['header']['typ_batman']
			ttl = yaml_conf['header']['ttl_batman']
			seq_id = yaml_conf['header']['seq_id_batman']
			forward_msg(typ,my_id,my_id,ttl,seq_id,0,0)
		
		if msg[0] == 'chat':
			# chat app .....
			print " i am sending chat"
			typ = yaml_conf['header']['typ_chat']
			ttl = yaml_conf['header']['ttl_chat']
			seq_id = yaml_conf['header']['seq_id_chat']
			recv_id = find_rout(msg[1])   #search for receiver in the routing table if not exit broadcast.
			print "receiver is %d" % (recv_id)
			forward_msg(typ,my_id,msg[1],ttl,seq_id,recv_id,msg[2])
			
		
		if msg[0] == 'command':
			# chat app .....
			print " i am sending chat"
			typ = yaml_conf['header']['typ_command']
			ttl = yaml_conf['header']['ttl_command']
			seq_id = yaml_conf['header']['seq_id_command']	
			recv_id = find_rout(msg[1])   #search for receiver in the routing table if not exit broadcast.
			forward_msg(msg[0],my_id,msg[1],ttl,seq_id,recv_id,msg[2])
		"""
			add more condition for more apps and types & add new entries in yaml.config file as well for new type, ttl and seq_id
		"""
	except ValueError:
		print "garbage value"
# Process 5 updates




