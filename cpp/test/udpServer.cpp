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

#define BUFSIZE 1024
#define PORT 12345

using namespace std;

mutex queueMutex; // locking queue for receiving and issuing commands
list<char *> cmdQueue;

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
 */
void cmdIssuer() {
  while(1) {
    bool validData = false;
    queueMutex.lock();
    if(!cmdQueue.empty()) {
      char *data = (char *)cmdQueue.front();
      cmdQueue.pop_front();
      cout << "received message: " << data << endl;
      validData = true;
    }
    queueMutex.unlock();
  }
}

int main(int argc, char** argv) {
  
  char *ipAddr = argv[1];
  
  struct sockaddr_in myaddr; // our address
  struct sockaddr_in remaddr; // remote address
  socklen_t addrlen = sizeof(remaddr);
  int recvlen;
  int fd;
  unsigned char buf[BUFSIZE];

  // create udp socket

  if((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    cout << "could not create socket" << endl;
    return 0;
  }

  // bind the socket to any valid IP address and a specific port

  memset((char *)&myaddr, 0, sizeof(myaddr));
  myaddr.sin_family = AF_INET;

  inet_pton(AF_INET, ipAddr, &(myaddr.sin_addr));
  //myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  myaddr.sin_port = htons(PORT);

  if(bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
    cout << "bind failed" << endl;
    return 0;
  }

  int test = 0;
  thread ct(cmdIssuer);
  ct.detach();
  thread rt(recvThread, fd, remaddr);
  rt.join(); 

  /*
  while(1) {
    cout << "waiting on port " << PORT << endl;
    recvlen = recvfrom(fd, buf, BUFSIZE, 0, (struct sockaddr *)&remaddr, &addrlen);
    cout << "received " << recvlen << " bytes" << endl;
    if(recvlen > 0) {
      buf[recvlen] = 0;
      cout << "received message: " << buf << endl;
    }
  }
  */
}
