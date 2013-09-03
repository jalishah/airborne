import thread
import sys
from time import sleep
from msg_processor import chk_msgtype
from createmsg import create_header
from start_batman import start_batman
from receiver import *
from storage import empty_uniq_id_list

try:
	thread.start_new_thread(sub_to_mac, () )
	thread.start_new_thread(sub_to_app, () )
	thread.start_new_thread(start_batman, ())
	thread.start_new_thread(empty_uniq_id_list,())

except:
	print "Error: unable to start thread"

while True:
	sleep(1)
