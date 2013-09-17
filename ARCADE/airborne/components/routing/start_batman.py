from time import sleep
from createmsg import create_header

def start_batman():
	while 1:
		#print "batman started"
		create_header(['batman'])
		sleep (120)   
