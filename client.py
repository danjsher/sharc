import socket
import sys
import threading
import Queue
import spidev
import os
import time

spi = spidev.SpiDev()
spi.open(0,0)

def readChannel(channel):
    if((channel  > 7) or (channel < 0)):
        return -1
    adc = spi.xfer2([1,(8+channel)<<4, 0])
    data = ((adc[1] & 3) << 8) + adc[2]
    return data

host = sys.argv[1]

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

server_address = (host, 12345)
#server_address = ('192.168.23.1', 12345)
print >>sys.stderr, 'connecting to %s port %s' % server_address
sock.connect(server_address)

adc_readings = [None, None, None, None, None]

while True:
    for i in range(0,5):
        adc_readings[i] = readChannel(i)

    message = str(adc_readings[0]) + " " + str(adc_readings[1]) + " " + str(adc_readings[2]) + " " + str(adc_readings[3]) + " " + str(adc_readings[4])

    print message
    
    sock.send(message)
    sock.recv(1024)
    time.sleep(.01)

#while True:
    #message = raw_input("Type your message to send then press enter: ")
#sock.send(message)
#sock.recv(1024)

sock.close()

