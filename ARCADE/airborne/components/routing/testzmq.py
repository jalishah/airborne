from forwarder import *

while 1:
	pub_to_app_socket.send("%s hello" % (str(5)) )  
