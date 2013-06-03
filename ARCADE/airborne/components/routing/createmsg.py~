from lookup import Lookup
from storage import *
from forwarder import *


def create_header(msg):

	my_id = yaml_conf['general']['my_id']	
	try:
		if msg[0] == 'batman':
			print "sending batman"
			typ = yaml_conf['batman']['typ']
			ttl = yaml_conf['batman']['ttl']
			seq_id = yaml_conf['batman']['seq_id']
			forward_msg(typ,my_id,my_id,ttl,seq_id,0,0)
		
		if msg[0] == 'chat':
			# chat app .....
			print " i am sending chat"
			typ = yaml_conf['chat']['typ']
			ttl = yaml_conf['chat']['ttl']
			seq_id = yaml_conf['chat']['seq_id']
			recv_id = find_rout(msg[1])   #search for receiver in the routing table if not exit broadcast.
			print "receiver is %d" % (recv_id)
			forward_msg(typ,my_id,msg[1],ttl,seq_id,recv_id,msg[2])
			print "chat send"
		
		if msg[0] == 'command':
			# chat app .....
			print " i am sending command"
			typ = yaml_conf['command']['typ']
			ttl = yaml_conf['command']['ttl']
			seq_id = yaml_conf['command']['seq_id']	
			recv_id = find_rout(msg[1])   #search for receiver in the routing table if not exit broadcast.
			forward_msg(typ,my_id,msg[1],ttl,seq_id,recv_id,msg[2])
			print "command send"
		"""
			add more condition for more apps and types & add new entries in yaml.config file as well for new type, ttl and seq_id
		"""
	except ValueError:
		print "garbage value"
# Process 5 updates




