import msgpack
import ../routing.rout
import serial
import thread
import yaml
import zmq
import random

hbcount = 0


def heartbeat():
	print "hello I am starting heart beat # ", hbcount
	hbcount += 1
	# heart beat will gather some sensor data e.g GPS data and will send it after every specific time interval
	rout(0x11,0x01,"hello")

while True:
	sleep(2)
	
