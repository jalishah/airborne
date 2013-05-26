import msgpack
import serial
import thread
import yaml
import zmq
import sys
from time import sleep

from storage import *
from forwarder import *
from createmsg import create_msg
from start_batman import start_batman

# config.YAML 


print "Collecting updates from nodes"

unp = msgpack.Unpacker()

"""
check the type of message 
Validation is done in this part 
control is transfered to storage module in case of batman
Unique id list is updated
Msg to publish application layer if receiver id matches
MSg is forward to Module forwader in case of batman if ttl is not 0
Msg is forward to Module forwader in case the receiver is different  

"""
def chk_msgtype ( msg ):
	global key_value;
	key_value = my_id      # to add the self id or receiver id in the key part of rout (dict)
	chk_rout_key(key_value)   # this will be call once in the begining when it receive its first msg (msg of any type)
	add_new_rout_values(key_value, msg[0][1])   #add this sender to the key(this receiver)
	
	if msg[0][0] == 4:
		key_value = msg[0][1]  # to add the sender id in the value part of rout (dict)
		chk_rout_key(key_value)		# to add the sender id as a key if not exist

		if key_value is not msg [0][2]:		# to avoid connecting same id's incase of sender and org is same
			add_new_rout_values(key_value,msg[0][2])
		print unique_ids_list
		chk_value = unique_ids_list.count(msg[0][4])
		print chk_value
		
		if chk_value == 0 :   # chk the unq_msg_id in list and operate only if not available
			unique_ids_list.append(msg[0][4])	
			print unique_ids_list
			if msg[0][3] != 0 and msg[0][2] != my_id:   # if ttl is not zero and if the receiver is not originator
				new_ttl = msg[0][3] - 1
				originator = msg[0][2]
				unq_id = msg[0][4]
				forward_msg(msg[0][0],my_id,originator,new_ttl,unq_id,0)  # forward the batman rout msg or broadcast again after decrement of ttl by one

	if msg[0][0] == 3:
		pub_to_app_socket.send(msg[1])  		
	return

def sub_to_mac():
	while 1:
		string = sub_to_mac_socket.recv()
		unp.feed(string)    # unpack the msg 
	    	for msg in unp:
			if type(msg) is tuple:
				print msg
				chk_msgtype(msg)   # check the msg type  
		get_rout()


try:
	thread.start_new_thread(sub_to_mac, () )
	thread.start_new_thread(create_msg, () )
	thread.start_new_thread(start_batman, ())
	thread.start_new_thread(empty_uniq_id_list,())

except:
	print "Error: unable to start thread"

while True:
	sleep(1)


#if __name__ == "__main__":

