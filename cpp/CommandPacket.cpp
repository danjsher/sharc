#include "CommandPacket.h"

CommandPacket::CommandPacket() {

  thumbADC  = 0;
  indexADC  = 0;
  middleADC = 0;
  ringADC   = 0;
  pinkyADC  = 0;

  bicepYaw   = 0;
  bicepPitch = 0;
  bicepRoll  = 0;
  
}

CommandPacket::CommandPacket(float packet[]) {

  thumbADC  = packet[0];
  indexADC  = packet[1];
  middleADC = packet[2];
  ringADC   = packet[3];
  pinkyADC  = packet[4];

  bicepYaw   = packet[5];
  bicepPitch = packet[6];
  bicepRoll  = packet[7];

}

CommandPacket::~CommandPacket() {
};

float CommandPacket::getPinkyADC() {
  return pinkyADC;
}

float CommandPacket::getRingADC() {
  return ringADC;
}

float CommandPacket::getMiddleADC() {
  return middleADC;
}

float CommandPacket::getIndexADC() {
  return indexADC;
}

float CommandPacket::getThumbADC() {
  return thumbADC;
}

float * CommandPacket::getHandADC(){
  float *ret = (float *)malloc(sizeof(float)*5);

  ret[0] = thumbADC;
  ret[1] = indexADC;
  ret[2] = middleADC;
  ret[3] = ringADC;
  ret[4] = pinkyADC;
  
  return ret;
}

float CommandPacket::getBicepYaw() {
  return bicepYaw;
}

float CommandPacket::getBicepPitch(){
  return bicepPitch;
}

float CommandPacket::getBicepRoll(){
  return bicepRoll;
}

float * CommandPacket::getBicepYpr(){
  float *ret = (float *)malloc(sizeof(float)*3);

  ret[0] = bicepYaw;
  ret[1] = bicepPitch;
  ret[2] = bicepRoll;

  return ret;
}
