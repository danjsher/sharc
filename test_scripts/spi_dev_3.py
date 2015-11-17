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
    try:
        while True:
            val = readChannel(7)
            print "ADC Result: ", str(val)
            time.sleep(.01)
    except KeyboardInterrupt:
        spi.close()
        sys.exit(0)
        
    
