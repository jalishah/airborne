from dev.cdc import CDC_nRF
import sys
import time

myDev=CDC_nRF(sys.argv[1])
myDev.setPower(1)
#myDev.setRX_Address(0,[0xc0,0x1d,0xbe,0xef,0x01])
#myDev.setTX_Address([0xc0,0x1d,0xbe,0xef,0x01])

myDev._bus.setDTR(True)
w=0
while True:
	w=w+1
	#time.sleep(0.1)
	print myDev._bus.write('This message! (%i)\n\r'%w)
