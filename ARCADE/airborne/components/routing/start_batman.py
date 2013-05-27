from time import sleep
from createmsg import create_header

def start_batman():
	while 1:
		print "batman started"
		create_header([4,0,0])
		sleep (60)   
