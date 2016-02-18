#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <string>
#include <mutex>
#include <list>
#include <arpa/inet.h>
#include "CommandPacket.h"

using namespace std;

std::mutex queueMutex;

std::list<float *> cmdQueue;

int parse_packet(char* input, float* output, int len) {
  char *str_token = strtok(input, " ");

  int i = 0;

  while(str_token != NULL && i < len) {
    output[i++] = atof(str_token);
    str_token = strtok(NULL, " ");
  }

  return i;
}

void clientThread(int clientHandle) {
  char buffer[1024];
  int rcvlen = 0;
  while(rcvlen = read(clientHandle, buffer, 1024)) {
    float rxData[8] = {0};

    int stat = parse_packet(buffer, rxData, 8);

    // put data on queue
    // cout << "locking queue" << endl;
    queueMutex.lock();
    cmdQueue.push_back(rxData);
    // cout << "Data pushed onto queue, unlocking" << endl;
    queueMutex.unlock();
    /*
    for(int i = 0; i < 8; i++) 
      cout << "rx data " << rxData[i] << endl;
    */
  }
}

float calcFingerPwm(float adcVal, float adcMin, float adcMax, float pwmMin, float pwmMax) {
  
  if(adcVal < adcMin) {
    return 20.0*pwmMax;
  } else if (adcVal > adcMax) {
    return (pwmMax-pwmMin)*20.0;
  }

  float  pulseWidth = (pwmMax - pwmMin*((adcVal - adcMin)/(adcMax - adcMin)))*20.0;

  return pulseWidth;
    
}

void cmdIssuer() {
  while(1) {
    cout << "Hi, I'm the command issuer thread." << endl;
    bool validData = false;
    float *readings;
    queueMutex.lock();
    cout << "Dequeing next command" << endl;
    if(!cmdQueue.empty()) {
      readings = cmdQueue.front();
      cmdQueue.pop_front();
      validData = true;
    }
    queueMutex.unlock();

    if(validData) {
      // Calculate pulse widths based on adc values from sleeve
      float thumbPwm  = calcFingerPwm(readings[0], 725, 1023, 7.5, 11.5);
      float indexPwm  = calcFingerPwm(readings[1], 745, 930, 8.0, 11.5);
      float middlePwm = calcFingerPwm(readings[2], 730, 1023, 8.5, 12.5);
      float ringPwm   = calcFingerPwm(readings[3], 680, 1023, 6.4, 10.4);
      float pinkyPwm  = calcFingerPwm(readings[4], 735, 1023, 6.0, 11.0);
      
      // construct echo commands
      
      string thumbCmd  = "echo 0=" + to_string(thumbPwm) + " > /dev/servoblaster";
      string indexCmd  = "echo 1=" + to_string(indexPwm) + " > /dev/servoblaster";
      string middleCmd = "echo 2=" + to_string(middlePwm) + " > /dev/servoblaster";
      string ringCmd   = "echo 3=" + to_string(ringPwm) + " > /dev/servoblaster";
      string pinkyCmd  = "echo 4=" + to_string(pinkyPwm) + " > /dev/servoblaster";


      cout << thumbCmd << endl << indexCmd << endl << middleCmd << endl << ringCmd << endl << pinkyCmd << endl;

      system(thumbCmd.c_str());
      system(indexCmd.c_str());
      system(middleCmd.c_str());
      system(ringCmd.c_str());
      system(pinkyCmd.c_str());
      
    } else {
      cout << "no data" << endl;
    }
    /*
    while(!cmdQueue.empty()){
      float *tmp = cmdQueue.front();
      cmdQueue.pop_front();
      cout << tmp[0] << endl;
    }
    queueMutex.unlock();
    */
    sleep(.06);
  }
}

int main(int argc, char **argv) {


  if(argc != 2) {
    cout << "invalid # of args" << endl << "usage: ./server <host_ip_addr>" << endl;
    exit(0);
  }

  cout << argv[1] << endl;
  char *ipAddr = argv[1];
  
  int listenPort = 12345;

  int socketHandle = socket(AF_INET, SOCK_STREAM, 0);
  if(!socketHandle) {
    printf("Error creating socket\n");
    exit(1);
  }

  struct sockaddr_in myaddr;
  memset(&myaddr, 0, sizeof(struct sockaddr_in));
  myaddr.sin_family = AF_INET;
  myaddr.sin_port = htons(listenPort);
  inet_pton(AF_INET, ipAddr, &(myaddr.sin_addr));
  //myaddr.sin_addr.s_addr = 

  int success = bind(socketHandle,  (struct sockaddr*) &myaddr, sizeof(myaddr));

  if(success < 0) {
    cout << "Error binding socket\n";
    exit(1);
  }
  
  // Set linger timeout to 0 so socket closes on program exit
  // not sure if this is necessary...
  struct linger linger_opt = {1, 0}; //Linger active, timeout
  setsockopt(socketHandle, SOL_SOCKET, SO_LINGER, &linger_opt, sizeof(linger_opt));


  success = listen(socketHandle, 1);
  if(success < 0) {
    cout << "Error while listening on socket";
    exit(1);
  }


  /* Dispatching command issuer thread */
  thread ci = thread(cmdIssuer);
  ci.detach();
  
  while(1) {
    struct sockaddr_in clientaddr;
    socklen_t clientaddr_len;
    int client = accept(socketHandle, (struct sockaddr*) &clientaddr, &clientaddr_len);
    if(client < 0) {
      cout << "Error accepting client\n";
      exit(1);
    }
    
    thread t = thread(clientThread, client);
    t.detach();
  }
}
