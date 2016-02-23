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


// Servoblaster pin assignments
#define THUMB  0
#define INDEX  5
#define MIDDLE 6
#define RING   3
#define PINKY  7

#define SHOULDER_ROTATION 8
#define SHOULDER_FLEXION  9

// network parameters
#define BUFSIZE 1024
#define PORT 12345

//settings for armMode
#define REALTIME 1
#define PLAYBACK 0

#define STOP 0
#define RECORD 1
#define PLAY 2

using namespace std;

int servoPins[7] = {
  THUMB,
  INDEX,
  MIDDLE,
  RING,
  PINKY,
  SHOULDER_ROTATION,
  SHOULDER_FLEXION
};

// struct for storing parsed data
typedef struct SleevePacket {
  int   packetNum;
  int   fingerVals[5];
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
	   << pkt->fingerVals[0] << " "
	   << pkt->fingerVals[1] << " "
	   << pkt->fingerVals[2] << " "
	   << pkt->fingerVals[3] << " "
	   << pkt->fingerVals[4] << " " 
	   << pkt->bicepYpr[0] << " " 
	   << pkt->bicepYpr[1] << " "
	   << pkt->bicepYpr[2]<< endl;
      

      
      // calculate each pwm signal
      float pwmSigs[7] = {0.0,
			  calcFingerPwm(pkt->fingerVals[1], 745, 930, 8.0, 11.5),
			  calcFingerPwm(pkt->fingerVals[2], 730, 1023, 9.5, 13.0),
			  calcFingerPwm(pkt->fingerVals[3], 680, 1023, 6.4, 10.4),
			  calcFingerPwm(pkt->fingerVals[4], 735, 1023, 7.0, 13.0),
			  0.0, //shoulder rotation
			  0.0}; //shoulder flexion
      
      // thumb calculation
      pwmSigs[0] = 11.5*((pkt->fingerVals[0] - 725.0)/(1023.0 - 725.0))*20.0;

      // shoulder rotation calc
      if( pkt->bicepYpr[2] < 0.0) {
	pwmSigs[5] = pkt->bicepYpr[2] + 130.0;
      } else if (pkt->bicepYpr[2] > 50.0 && pkt->bicepYpr[2] < 60.0 ) {
	pwmSigs[5] = pkt->bicepYpr[2] + 180.0;
      } else if (pkt->bicepYpr[2] > 0.0 && pkt->bicepYpr[2] < 50.0) {
	pwmSigs[5] = pkt->bicepYpr[2] + 140.0;
      }

      float increasingPitch = 1.0;

      if(pkt->bicepYpr[1] > 0.0 && pkt->bicepYpr[1]  < 70.0 && (pkt->bicepYpr[1] - prevPitch)  > 0.0) {
	pwmSigs[6] = pkt->bicepYpr[1] + 70.0;
      } else if (pkt->bicepYpr[1] >= 70.0) {
	pwmSigs[6] = 140.0;
	cout << "max pitch reached" << endl;
      } else if ( pkt->bicepYpr[1] <= 0.0 ) {
	pkt->bicepYpr[1] = 70.0;
      }

      

      
      string cmds[7];

      for(int i = 0; i < 7; i++) {
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

float calcFingerPwm(int adcVal, float adcMin, float adcMax, float pwmMin, float pwmMax) {

  if(adcVal < adcMin) {
    return 20*pwmMax;
  } else if (adcVal > adcMax) {
    return (pwmMax-pwmMin)*20.0;
  }

  float pulseWidth = (pwmMax - pwmMin*((adcVal - adcMin)/(adcMax - adcMin)))*20.0;

  return pulseWidth;
}

int parsePacket(char *input, SleevePacket *pkt) {

  char *strToken = strtok(input, " "); // split on space

  int i = 0;

  // first item is packet #
  pkt->packetNum = atoi(strToken);

  // next 5 items are finger adc values thumb = 0, index = 1, etc...

  for(i = 0; i < 5; i++) {
    strToken = strtok(NULL, " ");
    pkt->fingerVals[i] = atoi(strToken);
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

  // dispatch command issuer thread
  thread ct(cmdIssuer);
  ct.detach();

  //dispatch receiver thread
  thread rt(recvThread, udpSocket, remaddr);
  rt.join(); 

}
