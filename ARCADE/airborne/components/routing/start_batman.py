from time import sleep
from forwarder import forward_msg
from storage import *


def start_batman():
	unq_id = 6
	global my_id
	while 1:
		print "batman started"
		forward_msg(4,my_id,3,unq_id)
		unq_id += 1		
		sleep (30)   
