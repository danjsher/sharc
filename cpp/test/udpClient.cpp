#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define SERVICE_PORT 12345

#define BUFLEN 2048
#define MSGS 5	/* number of messages to send */

int main(int argc, char** argv)
{
  char *ipAddr = argv[1];
  
  struct sockaddr_in myaddr, remaddr;
  int fd, i, slen=sizeof(remaddr);
  char buf[BUFLEN];	/* message buffer */
  int recvlen;		/* # bytes in acknowledgement message */
  char *server = "127.0.0.1";	/* change this to use a different server */
  
  /* create a socket */
  
  if ((fd=socket(AF_INET, SOCK_DGRAM, 0))==-1)
    printf("socket created\n");
  
  /* bind it to all local addresses and pick any port number */
  
  memset((char *)&myaddr, 0, sizeof(myaddr));
  myaddr.sin_family = AF_INET;
  myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  myaddr.sin_port = htons(0);
  
  if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
    perror("bind failed");
    return 0;
  }       
  
  /* now define remaddr, the address to whom we want to send messages */
  /* For convenience, the host address is expressed as a numeric IP address */
  /* that we will convert to a binary format via inet_aton */
  
  memset((char *) &remaddr, 0, sizeof(remaddr));
  remaddr.sin_family = AF_INET;
  remaddr.sin_port = htons(SERVICE_PORT);
  inet_pton(AF_INET, ipAddr, &(remaddr.sin_addr));
  /*
    if (inet_aton(server, &remaddr.sin_addr)==0) {
    fprintf(stderr, "inet_aton() failed\n");
    exit(1);
	}
  */
  /* now let's send the messages */


  printf("Sending packet %d to %s port %d\n", i, ipAddr, SERVICE_PORT);
  for (i=0; i < MSGS; i++) {
    sprintf(buf, "This is packet %d", i);
    if (sendto(fd, buf, strlen(buf), 0, (struct sockaddr *)&remaddr, slen)==-1) {
      perror("sendto");
      exit(1);
    }
    /* now receive an acknowledgement from the server 
    recvlen = recvfrom(fd, buf, BUFLEN, 0, (struct sockaddr *)&remaddr, (socklen_t *)&slen);
    if (recvlen >= 0) {
    buf[recvlen] = 0;	// expect a printable string - terminate it 
      printf("received message: \"%s\"\n", buf);
    }
  */
  }
  return 0;
}
