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
#include "CommandPacket.h"
#include "ThreadQueue.h"

using namespace std;

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
    cout << "Received: \"" << rcvlen << "\" from client" << endl;

    float rxData[8] = {0};

    int stat = parse_packet(buffer, rxData, 8);

    cout << "First element " << rxData[0] << endl;
    
    if(strcmp(buffer, "quit") == 0) {
      cout << "Received quit command. Exiting..." << endl;
      return;
    }
  }
}

void cmdIssuer() {
  while(1) {
    cout << "Hi, I'm the command issuer thread." << endl;
    sleep(3);
  }
}

int main(int argc, char **argv) {

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
  myaddr.sin_addr.s_addr = htonl(INADDR_ANY);

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
