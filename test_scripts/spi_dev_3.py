#!/usr/bin/python

import time
import sys
import spidev
import os

spi = spidev.SpiDev()
spi.open(0,0)

def readChannel(channel):
    if((channel > 7) or (channel < 0)):
        return -1
    adc = spi.xfer2([1,(8+channel)<<4,0])
    data = ((adc[1]&3)<<8) + adc[2]
    return data


if __name__ == '__main__':
    list = [None, None, None, None, None]
    
    try:
        while True:
            for i in range(0,5):
                list[i] = readChannel(i)
            print "| %s | %s | %s | %s | %s |" % (list[0], list[1], list[2], list[3], list[4])
            time.sleep(.01)
    except KeyboardInterrupt:
        spi.close()
        sys.exit(0)
        
    
