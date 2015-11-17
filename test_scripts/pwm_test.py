#!/usr/bin/python

import time
import os

STEP = 100
DELAY = 0.5

def pwm(servo, angle):
    cmd = "echo " + str(servo) + "=" + str(angle) + "% > /dev/servoblaster"
    os.system(cmd)
    time.sleep(0.1)

while True:
    pwm(1, 100)
    time.sleep(1)
    pwm(1,0)
    time.sleep(1)
                
