#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <cstdlib>
#include <iostream>

using namespace std;

int main(int argc, char **argv) {
  int socketHandle = socket(AF_INET, SOCK_STREAM, 0);
  if(socketHandle < 0) {
    cout << "Cannot create socket";
    exit(1);
  }

  struct sockaddr_in clientaddr;
  int clientaddr_len;
  memset(&clientaddr, 0, sizeof(clientaddr));
  char* clientHost = "localhost";
  
  struct hostent *host = gethostbyname(clientHost);

  if(host == NULL) {
    cout<< "Cannout resolve host address";
    exit(1);
  }

  clientaddr.sin_family = AF_INET;
  int clientPort = 12345;
  clientaddr.sin_port = htons(clientPort);

  memmove(&(clientaddr.sin_addr.s_addr), host->h_addr_list[0], 4);

  int success = connect(socketHandle,  (struct sockaddr*) &clientaddr, sizeof(clientaddr));
  if(success < 0) {
    cout << "Cannot connect to server";
    exit(1);
  }

  cout << "Connected to server";
  
  char buffer[1024];
  success = read(socketHandle, buffer, 1024);
  if(success < 0) {
    cout << "Error receving";
    exit(1);
  }
  cout<< "Received " << buffer
  write(socketHandle, "testing", 7);

  close(socketHandle);
  
}
