import msgpack
from interface import Interface
from time import sleep
import serial
i = Interface('/dev/ttyACM0')
c = 0
while True:
	mFlag = 0	
	mType = 'b'
	senderID = 5
	speed = 4.2	
	heading = 360
	myMessage = msgpack.packb([mFlag,mType,senderID,speed,heading,c])
	c += 1	 	
	i.send(myMessage)
	sleep(0.1)
	
