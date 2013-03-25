import msgpack
import rout
from time import sleep
import serial
import thread
import yaml
import zmq
import random

hbcount = 0


def main():
	while 1:	
		global hbcount	
		print "hello I am starting heart beat # " , hbcount
		hbcount += 1
		# heart beat will gather some sensor data e.g GPS data and will send it after every specific time interval
		rout.routmsg(0x01,0x11,"hello")
		sleep(0.1)

if __name__ == "__main__":
    main()
