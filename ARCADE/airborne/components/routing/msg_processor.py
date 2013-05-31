from storage import *
from forwarder import *
from storage import *
import yaml

key_value = 0

"""
check the type of message 
Validation is done in this part 
control is transfered to storage module in case of batman
Unique id list is updated
Msg to publish application layer if receiver id matches
MSg is forward to Module forwader in case of batman if ttl is not 0
Msg is forward to Module forwader in case the receiver is different  
in case of batman routing ... the 3rd position is for originator in case of other msgz its for target node id
"""

# to add the self id or receiver id in the key part of rout (dict)
chk_rout_key(yaml_conf['header']['my_id'])   # this will be call once in the begining

def chk_msgtype ( msg ):
		
	global key_value;
	my_id = doc['header']['my_id']

	try:
		add_new_rout_values(my_id, msg[0][1])   #add this new node to self key (this receiver)
	
		if (msg[0][3]) != 0  :
			unq_id = msg[0][4]
			new_ttl = msg[0][3] - 1

			if msg[0][0] == 4:   # for type batman only
				key_value = msg[0][1]  # to add the sender id in the value part of rout (dict)
				chk_rout_key(key_value)		# to add the sender id as a key if not exist

				if key_value is not msg [0][2]:		# to avoid connecting same id's incase of sender and org is same
					add_new_rout_values(key_value,msg[0][2])
				print unique_ids_list
				chk_value = unique_ids_list.count(msg[0][4])
				if chk_value == 0 :   # chk the unq_msg_id in list and operate only if not available
					unique_ids_list.append(msg[0][4])	
					if msg[0][3] != 0 and msg[0][2] != my_id:   # if ttl is not zero and if the receiver is not originator
						originator = msg[0][2]
						forward_msg(msg[0][0],my_id,originator,new_ttl,unq_id,0,0)  # forward the batman rout msg or broadcast again after decrement of ttl by one

			if msg[0][0] != 4 and msg[0][2] != my_id :   # if type is not batman and target id is not my_id and recv_id = 0
				recv_id = find_rout_key(msg[0][2])   #search for receiver in the routing table if not exit broadcast.
				print recv_id
				forward_msg(msg[0][0],my_id,msg[0][2],new_ttl,unq_id,recv_id,msg[1])

			if msg[0][0] != 4 and msg[0][2] == my_id:
				pub_to_app_socket.send("%s %s" % (str(msg[0][0]),msg[1]) )	
	except TypeError:
		print "garbage value"
	return
