#!/usr/bin/env python

import time
import os
import RPi.GPIO as GPIO

GPIO.setmode(GPIO.BCM)
DEBUG = 1

def readAdc(channel, clockPin, mosiPin, misoPin, csPin):
    if((channel > 7) or (channel < 0)):
        return -1
    GPIO.output(csPin, True)

    GPIO.output(clockPin, False)
    GPIO.output(csPin, False)

    commandout = channel
    commandout |= 0x18
    commandout <<= 3

    for i in range(5):
        if(commandout & 0x80):
            GPIO.output(mosiPin, True)
        else:
            GPIO.output(mosiPin,False)
        commandout <<= 1
        GPIO.output(clockPin, True)
        GPIO.output(clockPin, False)


    adcOut = 0

    for i in range(12):
        GPIO.output(clockPin, True)
        GPIO.output(clockPin, False)
        adcOut <<= 1
        if(GPIO.input(misoPin)):
            adcOut |= 0x1

    GPIO.output(csPin, True)

    adcOut >>= 1
    return adcOut

SPICLK = 23
SPIMISO = 21
SPIMOSI = 19
SPICS = 24

GPIO.setup(SPIMOSI, GPIO.OUT)
GPIO.setup(SPIMISO, GPIO.IN)
GPIO.setup(SPICLK, GPIO.OUT)
GPIO.setup(SPICS, GPIO.OUT)

adcValue = 0;

while True:

    adcValue = readAdc(2, SPICLK, SPIMOSI, SPIMISO, SPICS)
    print "adcValue is ", str(adcValue)
    time.sleep(1)
    

            
    
