#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <math.h>

#include <wiringPi.h>
#include "../MPU/I2Cdev.h"
#include "../AdafruitPwm.h"

/*
#define MODE1         0x00
#define MODE2         0x01
#define SUBADR1       0x02
#define SUBADR2       0x03
#define SUBADR3       0x04
#define PRESCALE      0xFE
#define LED0_ON_L     0x06
#define LED0_ON_H     0x07
#define LED0_OFF_L    0x08
#define LED0_OFF_H    0x09
#define ALL_LED_ON_L  0xFA
#define ALL_LED_ON_H  0xFB
#define ALL_LED_OFF_L 0xFC
#define ALL_LED_OFF_H 0xFD

#define RESTART 0x80
#define SLEEP   0x10
#define ALLCALL 0x01
#define INVRT   0x10
#define OUTDRV  0x04

#define PWM_ADDR 0x40

void init(uint8_t address) {
  
  I2Cdev::writeByte(address, MODE2, OUTDRV);
  I2Cdev::writeByte(address, MODE1, ALLCALL);
  sleep(0.005);

  uint8_t data;
  uint8_t mode1 = I2Cdev::readByte(PWM_ADDR, MODE1, &data);
  mode1 = mode1 & ~SLEEP;
  I2Cdev::writeByte(address, MODE1, mode1);
  sleep(0.005);
  
}

void setPWMFreq(int freq) {
  float prescaleval = 25000000.0;
  prescaleval /= 4096;
  prescaleval /= freq;
  prescaleval -= 1.0;

  float prescale = floor(prescaleval + 0.5);

  uint8_t data;
  uint8_t oldmode = I2Cdev::readByte(PWM_ADDR, MODE1, &data);
  uint8_t newmode = (oldmode & 0x7F) | 0x10;
  I2Cdev::writeByte(PWM_ADDR, MODE1, newmode);
  I2Cdev::writeByte(PWM_ADDR, PRESCALE, (uint8_t)prescale);
  I2Cdev::writeByte(PWM_ADDR, MODE1, oldmode);
  sleep(0.005);
  I2Cdev::writeByte(PWM_ADDR, MODE1, oldmode | 0x80);
  
}


void setPWM(uint8_t channel, uint16_t on, uint16_t off) {

  I2Cdev::writeByte(PWM_ADDR, LED0_ON_L + 4*channel, on & 0xFF); //lower byte
  I2Cdev::writeByte(PWM_ADDR, LED0_ON_H + 4*channel, on >> 8);   //upper byte
  I2Cdev::writeByte(PWM_ADDR, LED0_OFF_L + 4*channel, off & 0xFF);  
  I2Cdev::writeByte(PWM_ADDR, LED0_OFF_H + 4*channel, off >> 8);
  
}

void setAllPWM(uint16_t on, uint16_t off) {

  I2Cdev::writeByte(PWM_ADDR, ALL_LED_ON_L, on & 0xFF); //lower byte
  I2Cdev::writeByte(PWM_ADDR, ALL_LED_ON_H, on >> 8);   //upper byte
  I2Cdev::writeByte(PWM_ADDR, ALL_LED_OFF_L, off & 0xFF);  
  I2Cdev::writeByte(PWM_ADDR, ALL_LED_OFF_H, off >> 8);
  
}
*/
int main(int argc, char **argv) {

  AdafruitPwm pwm = AdafruitPwm();
  pwm.setPWMFreq(60);
  while(1){
    pwm.setPWM(0, 0, 150);
    sleep(1);
    pwm.setPWM(0, 0, 600);
    sleep(1);
  }
  
  
}
