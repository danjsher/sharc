#include <netinet/in.h>
#include <stdlib.h>
#include <iostream>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>

#define BUFSIZE 2048
#define PORT 12345

using namespace std;

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

  while(1) {
    cout << "waiting on port " << PORT << endl;
    recvlen = recvfrom(fd, buf, BUFSIZE, 0, (struct sockaddr *)&remaddr, &addrlen);
    cout << "received " << recvlen << " bytes" << endl;
    if(recvlen > 0) {
      buf[recvlen] = 0;
      cout << "received message: " << buf << endl;
    }
  }
}
