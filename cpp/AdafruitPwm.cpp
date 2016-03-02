#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <math.h>

#include <wiringPi.h>
#include "MPU/I2Cdev.h"
#include "AdafruitPwm.h"

/*
 * Constructor initializes PWM controller
 */
AdafruitPwm::AdafruitPwm() {
  I2Cdev::writeByte(DEFAULT_ADDR, MODE2, OUTDRV);
  I2Cdev::writeByte(DEFAULT_ADDR, MODE1, ALLCALL);
  sleep(0.005);

  uint8_t data;
  uint8_t mode1 = I2Cdev::readByte(DEFAULT_ADDR, MODE1, &data);
  mode1 = mode1 & ~SLEEP;
  I2Cdev::writeByte(DEFAULT_ADDR, MODE1, mode1);
  sleep(0.005);
}

/*
 * Set frequency in Hz
 */
void AdafruitPwm::setPWMFreq(int freq) {
  float prescaleval = 25000000.0;
  prescaleval /= 4096;
  prescaleval /= freq;
  prescaleval -= 1.0;

  float prescale = floor(prescaleval + 0.5);

  uint8_t data;
  uint8_t oldmode = I2Cdev::readByte(DEFAULT_ADDR, MODE1, &data);
  uint8_t newmode = (oldmode & 0x7F) | 0x10;
  I2Cdev::writeByte(DEFAULT_ADDR, MODE1, newmode);
  I2Cdev::writeByte(DEFAULT_ADDR, PRESCALE, (uint8_t)prescale);
  I2Cdev::writeByte(DEFAULT_ADDR, MODE1, oldmode);
  sleep(0.005);
  I2Cdev::writeByte(DEFAULT_ADDR, MODE1, oldmode | 0x80);
}

/*
 * Set PWM signal on given channel
 * uint16_t on: number from 0..4096 determines the % of the duty
 *              cycle the pulse will go high (typically 0 to
 *              start high)
 * uint16_t off: number from 0..4096 determines the % of the duty
 *               cycle that the pulse will go low
 */
void AdafruitPwm::setPWM(int channel, uint16_t on, uint16_t off) {
  I2Cdev::writeByte(DEFAULT_ADDR, LED0_ON_L + 4*channel, on & 0xFF); //lower byte
  I2Cdev::writeByte(DEFAULT_ADDR, LED0_ON_H + 4*channel, on >> 8);   //upper byte
  I2Cdev::writeByte(DEFAULT_ADDR, LED0_OFF_L + 4*channel, off & 0xFF);  
  I2Cdev::writeByte(DEFAULT_ADDR, LED0_OFF_H + 4*channel, off >> 8);
}

/*
 * Set pwm signal on all channels
 */
void AdafruitPwm::setAllPWM(uint16_t on, uint16_t off) {
  I2Cdev::writeByte(DEFAULT_ADDR, ALL_LED_ON_L, on & 0xFF); //lower byte
  I2Cdev::writeByte(DEFAULT_ADDR, ALL_LED_ON_H, on >> 8);   //upper byte
  I2Cdev::writeByte(DEFAULT_ADDR, ALL_LED_OFF_L, off & 0xFF);  
  I2Cdev::writeByte(DEFAULT_ADDR, ALL_LED_OFF_H, off >> 8);
}
