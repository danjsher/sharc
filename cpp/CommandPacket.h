#ifndef COMMANDPACKET_H
#define COMMANDPACKET_H

#include <cstdio>
#include <iostream>
#include <stdlib.h>

using namespace std;

class CommandPacket {
 public:
  CommandPacket();
  CommandPacket(float packet[]);

  ~CommandPacket();
  
  float getPinkyADC();
  float getRingADC();
  float getMiddleADC();
  float getIndexADC();
  float getThumbADC();

  float * getHandADC();
  
  float getBicepYaw();
  float getBicepPitch();
  float getBicepRoll();

  float * getBicepYpr();

 private:
  float pinkyADC;
  float ringADC;
  float middleADC;
  float indexADC;
  float thumbADC;

  float bicepYaw;
  float bicepPitch;
  float bicepRoll;
};

#endif




