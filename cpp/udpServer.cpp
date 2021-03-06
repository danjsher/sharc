#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <list>
#include <mutex>
#include <thread>
#include <string>

#include "AdafruitPwm.h"

// Servoblaster pin assignments
#define THUMB  0
#define INDEX 4
#define MIDDLE 2
#define RING   3
#define PINKY  7

#define SHOULDER_ROTATION 2
#define SHOULDER_FLEXION  2

#define BICEP_FLEX 10

// network parameters
#define BUFSIZE 1024
#define PORT 12345

//settings for armMode
#define REALTIME 1
#define PLAYBACK 0

#define STOP 0
#define RECORD 1
#define PLAY 2

#define PWM_SIGS 8

using namespace std;

int servoPins[PWM_SIGS] = {
  THUMB,
  INDEX,
  MIDDLE,
  RING,
  PINKY,
  SHOULDER_ROTATION,
  SHOULDER_FLEXION,
  BICEP_FLEX
};

// struct for storing parsed data
typedef struct SleevePacket {
  int   packetNum;
  int   adcVals[6];
  float bicepYpr[3];
} SleevePacket;

void  recvThread(int, struct sockaddr_in);
void  cmdIssuer();
int   parsePacket(char *, SleevePacket*);
float calcFingerPwm(int adcVal, float adcMin, float adcMax, float pwmMin, float pwmMax);

mutex queueMutex; // locking queue for receiving and issuing commands
list<char *> cmdQueue; //queue for storing input

int armMode = REALTIME; //initially start in real time mode
int playBack = STOP;


/* 
 * global pwmController object, used by cmdIssuer 
 */
AdafruitPwm pwmController = AdafruitPwm();

/*
 * Thread for receving data from sleeve
 * Pushes data onto queue.
 * NOTE: Maybe unpackage here or leave that to the
 *       cmdIssuer so we can receive faster
 */
void recvThread(int socketHandle, struct sockaddr_in remaddr) {
  socklen_t addrlen;
  int recvlen;
  char buf[BUFSIZE];
  
  while(1) {
    //cout << "waiting on port " << PORT << endl;
    recvlen = recvfrom(socketHandle, buf, BUFSIZE, 0, (struct sockaddr *)&remaddr, &addrlen);
    //cout << "received " << recvlen << " bytes" << endl;
    if(recvlen > 0) {
      buf[recvlen] = 0;
      //cout << "received message: " << buf << endl;
    }
    
    queueMutex.lock();
    //cout << "pushing data onto queue" << endl;
    cmdQueue.push_back(buf);
    queueMutex.unlock();

    //acknowledge packet received so sleeve can sample again
    if(sendto(socketHandle, buf, strlen(buf), 0, (struct sockaddr *)&remaddr, addrlen) < 0) {
      cout << "send error" << endl;
      return;
    }       
  }
}

/*
 * Thread for issuing commands. 
 * Deques data from the receive queue and issues the commands with servoblaster
 * Unpackage data here to offload work from receive thread
 */
void cmdIssuer() {
  SleevePacket *pkt;
  char *data;
  bool validData = false; //only do calcs if data was found in queue

  float increasingPitch = 0.0;
  float prevPitch = 0.0;
  while(1) {
    queueMutex.lock();
    if(!cmdQueue.empty()) {
      data = (char *)cmdQueue.front();
      cmdQueue.pop_front();
      validData = true;
    }
    if(validData && ((playBack == PLAY && armMode == PLAYBACK) || (armMode == REALTIME))) {
      
      //put packet into pkt struct
      pkt = (SleevePacket *)malloc(sizeof(SleevePacket));
      parsePacket(data, pkt);
    
      
      cout << pkt->packetNum << " "
	   << pkt->adcVals[0] << " "
	   << pkt->adcVals[1] << " "
	   << pkt->adcVals[2] << " "
	   << pkt->adcVals[3] << " "
	   << pkt->adcVals[4] << " "
	   << pkt->adcVals[5] << " "
	   << pkt->bicepYpr[0] << " " 
	   << pkt->bicepYpr[1] << " "
	   << pkt->bicepYpr[2]<< endl;
      

      
      // calculate each pwm signal
      float pwmSigs[PWM_SIGS] = {0.0,
			  calcFingerPwm(pkt->adcVals[1], 745, 930, 7.2, 10.4),
			  calcFingerPwm(pkt->adcVals[2], 730, 1023, 7.4, 11.5),
		 	  calcFingerPwm(pkt->adcVals[3], 680, 1023, 7.0, 10.2),
			  calcFingerPwm(pkt->adcVals[4], 735, 1023, 5.3, 9.6),
			  0.0,  // shoulder rotation
			  0.0,  // shoulder flex
			  0.0}; // bicep flex
      
      // thumb calculation
      pwmSigs[0] = 11.4*((pkt->adcVals[0] - 725.0)/(1023.0 - 725.0))*20.0;


      // shoulder rotation calc
      if(pkt->bicepYpr[2] >= 0.0 && pkt->bicepYpr[2] < 60.0 ) { //rotation back
	pwmSigs[5] = -1.1667*pkt->bicepYpr[2] + 130.0;
      } else if (pkt->bicepYpr[2] < 0.0 && pkt->bicepYpr[2] > -80.0) { //forward rotation
	pwmSigs[5] = -1.375*pkt->bicepYpr[2] + 130.0;
      } else if ( pkt->bicepYpr[2] >= 60.0) { // out bounds back
	pwmSigs[5] = 60.0;
      } else if (pkt->bicepYpr[2] <= -80.0) { // out of bounds forward
	pwmSigs[5] = 240.0;
      }

      // shoulder flex calc
      if(pkt->bicepYpr[1] > -45.0 && pkt->bicepYpr[1] < 0.0) {
	pwmSigs[6] =  -1.4 * pkt->bicepYpr[1] + 70.0;
      } else if ( pkt->bicepYpr[1] <= -45.0) { // flex out bound
	pwmSigs[6] = 140.0;
      } else if (pkt->bicepYpr[1] >= 0.0) { // flex in bound
	pwmSigs[6] = 70.0;
      }

      // bicep flex calc
      pwmSigs[7] =  (3.0+4.5*((pkt->adcVals[5]-730.0)/(990.0-740.0)))*20.0;
      
      string cmds[PWM_SIGS];

      for(int i = 0; i < PWM_SIGS; i++) {
	cmds[i] = "echo " + to_string(servoPins[i]) + "=" + to_string(pwmSigs[i]) + " > /dev/servoblaster";
	cout << cmds[i] << endl;
	system(cmds[i].c_str());
      }
      
      //don't want mem leaks
      free(pkt);
      
    }
    queueMutex.unlock();
    validData = false;
  } // while(1)
}

// These work for index - pinky, thumb is calculated separately because servo was reversed

float calcFingerPwm(int adcVal, float adcMin, float adcMax, float pwmMin, float pwmMax) {

  if(adcVal < adcMin) {
    return 20*pwmMax;
  } else if (adcVal > adcMax) {
    return (pwmMax-pwmMin)*20.0;
  }

  float pulseWidth = (pwmMax - pwmMin*((adcVal - adcMin)/(adcMax - adcMin)))*20.0;

  return pulseWidth;
}

// Packet received from client is a space separated string
// Format:
// | Packet Number | Finger ADC Vals (5 of them) | Bicep ADC Val | Bicep Yaw | Bicep Pitch | Bicep Roll |

int parsePacket(char *input, SleevePacket *pkt) {

  char *strToken = strtok(input, " "); // split on space

  int i = 0;

  // first item is packet #
  pkt->packetNum = atoi(strToken);

  // next 6 items are adc values thumb = 0, index = 1, etc...
  // 5 = bicep adc index
  
  for(i = 0; i < 6; i++) {
    strToken = strtok(NULL, " ");
    pkt->adcVals[i] = atoi(strToken);
  }

  // next 3 will be floats for bicep yaw, pitch, roll respectively
  
  for( i = 0; i < 3; i++) {
    strToken = strtok(NULL, " ");
    cout << "token: " << strToken << endl;
    pkt->bicepYpr[i] = atof(strToken);
  }
  
  return 1;
  
}

int main(int argc, char** argv) {

  if(argc != 2) {
    cout << "wrong # of args" << endl << "usage: ./udpServer <server_ip_addr>" << endl;
    return 0;
  }
  
  char *ipAddr = argv[1]; // first argument in dot format
  
  struct sockaddr_in myaddr; // our address
  struct sockaddr_in remaddr; // remote address
  socklen_t addrlen = sizeof(remaddr);
  int recvlen;
  int udpSocket; //socket descriptor 

  // create udp socket
  if((udpSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    cout << "could not create socket" << endl;
    return 0;
  }

  // fill out sockaddr_in structure with host IP, host port and family (Always AF_INET for IP)
  memset((char *)&myaddr, 0, sizeof(myaddr));
  myaddr.sin_family = AF_INET;
  inet_pton(AF_INET, ipAddr, &(myaddr.sin_addr));
  myaddr.sin_port = htons(PORT);

  if(bind(udpSocket, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
    cout << "bind failed" << endl;
    return 0;
  }

  //set up pwmController frequency to 50 Hz
  pwmController.setPWMFreq(50);

  // dispatch command issuer thread
  cout << "Spawning command issuer thread..." << endl;
  thread ct(cmdIssuer);
  ct.detach();
  cout << "Done!" << endl;
  
  //dispatch receiver thread
  cout << "Spawning receiver thread..." << endl;
  thread rt(recvThread, udpSocket, remaddr);
  cout << "Done!" << endl;
  rt.join(); // wait for receiver thread to join to keep main running
}
