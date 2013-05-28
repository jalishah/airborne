from forwarder import *

while 1:
	comnd = raw_input("enter your short message:    ")
	pub_to_app_socket.send("%s %s" % (str(1) , comnd) ) 
