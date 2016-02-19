#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>
#include <unistd.h>
#include <math.h>
#include <sstream>

#include "MPU/I2Cdev.h"
#include "MPU/MPU6050_6Axis_MotionApps20.h"
#include "RaspberryPi-mcp3008Spi/mcp3008Spi.h"

#define SERVICE_PORT 12345

#define BUFLEN 2048
#define MSGS 5	/* number of messages to send */

using namespace std;

/************************************* MCP3008 INIT ******************************************/

  mcp3008Spi a2d("/dev/spidev0.0",SPI_MODE_0, 1000000, 8);

/************************************* MPU CODE **********************************************/

MPU6050 mpu;

#define OUTPUT_READABLE_QUATERNION

#define OUTPUT_READABLE_YAWPITCHROLL

// MPU control/status vars
bool dmpReady = false;  // set true if DMP init was successful
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer

// orientation/motion vars
Quaternion q;           // [w, x, y, z]         quaternion container
VectorInt16 aa;         // [x, y, z]            accel sensor measurements
VectorInt16 aaReal;     // [x, y, z]            gravity-free accel sensor measurements
VectorInt16 aaWorld;    // [x, y, z]            world-frame accel sensor measurements
VectorFloat gravity;    // [x, y, z]            gravity vector
float euler[3];         // [psi, theta, phi]    Euler angle container
float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

// packet structure for InvenSense teapot demo
uint8_t teapotPacket[14] = { '$', 0x02, 0,0, 0,0, 0,0, 0,0, 0x00, 0x00, '\r', '\n' };


// ================================================================
// ===                      INITIAL SETUP                       ===
// ================================================================

void dmpSetup() {
    // initialize device
    printf("Initializing I2C devices...\n");
    mpu.initialize();

    // verify connection
    printf("Testing device connections...\n");
    printf(mpu.testConnection() ? "MPU6050 connection successful\n" : "MPU6050 connection failed\n");

    // load and configure the DMP
    printf("Initializing DMP...\n");
    devStatus = mpu.dmpInitialize();
    
    // make sure it worked (returns 0 if so)
    if (devStatus == 0) {
        // turn on the DMP, now that it's ready
        printf("Enabling DMP...\n");
        mpu.setDMPEnabled(true);

        // enable Arduino interrupt detection
        //Serial.println(F("Enabling interrupt detection (Arduino external interrupt 0)..."));
        //attachInterrupt(0, dmpDataReady, RISING);
        mpuIntStatus = mpu.getIntStatus();

        // set our DMP Ready flag so the main loop() function knows it's okay to use it
        printf("DMP ready!\n");
        dmpReady = true;

        // get expected DMP packet size for later comparison
        packetSize = mpu.dmpGetFIFOPacketSize();
    } else {
        // ERROR!
        // 1 = initial memory load failed
        // 2 = DMP configuration updates failed
        // (if it's going to break, usually the code will be 1)
        printf("DMP Initialization failed (code %d)\n", devStatus);
    }
}

// ================================================================
// ===                    MAIN PROGRAM LOOP                     ===
// ================================================================

void dmpLoop(float *bicepYpr) {
    // if programming failed, don't try to do anything
    if (!dmpReady) return;
    // get current FIFO count
    fifoCount = mpu.getFIFOCount();

    if (fifoCount >= 1024) {
        // reset so we can continue cleanly
        mpu.resetFIFO();
        printf("FIFO overflow!\n");

    // otherwise, check for DMP data ready interrupt (this should happen frequently)
    } else if (fifoCount >= 42) {
        // read a packet from FIFO
        mpu.getFIFOBytes(fifoBuffer, packetSize);

	
        #ifdef OUTPUT_READABLE_QUATERNION
            // display quaternion values in easy matrix form: w x y z
            mpu.dmpGetQuaternion(&q, fifoBuffer);
            printf("quat %7.2f %7.2f %7.2f %7.2f    ", q.w,q.x,q.y,q.z);
        #endif

        #ifdef OUTPUT_READABLE_EULER
            // display Euler angles in degrees
            mpu.dmpGetQuaternion(&q, fifoBuffer);
            mpu.dmpGetEuler(euler, &q);
            printf("euler %7.2f %7.2f %7.2f    ", euler[0] * 180/M_PI, euler[1] * 180/M_PI, euler[2] * 180/M_PI);
        #endif
	
        #ifdef OUTPUT_READABLE_YAWPITCHROLL
            // display Euler angles in degrees
            mpu.dmpGetQuaternion(&q, fifoBuffer);
            mpu.dmpGetGravity(&gravity, &q);
            mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
            printf("ypr  %7.2f %7.2f %7.2f    ", ypr[0] * 180/M_PI, ypr[1] * 180/M_PI, ypr[2] * 180/M_PI);
	    bicepYpr[0] = ypr[0] * 180/M_PI;
	    bicepYpr[1] = ypr[1] * 180/M_PI;
	    bicepYpr[2] = ypr[2] * 180/M_PI;
        #endif
	    
        #ifdef OUTPUT_READABLE_REALACCEL
            // display real acceleration, adjusted to remove gravity
            mpu.dmpGetQuaternion(&q, fifoBuffer);
            mpu.dmpGetAccel(&aa, fifoBuffer);
            mpu.dmpGetGravity(&gravity, &q);
            mpu.dmpGetLinearAccel(&aaReal, &aa, &gravity);
            printf("areal %6d %6d %6d    ", aaReal.x, aaReal.y, aaReal.z);
        #endif

        #ifdef OUTPUT_READABLE_WORLDACCEL
            // display initial world-frame acceleration, adjusted to remove gravity
            // and rotated based on known orientation from quaternion
            mpu.dmpGetQuaternion(&q, fifoBuffer);
            mpu.dmpGetAccel(&aa, fifoBuffer);
            mpu.dmpGetGravity(&gravity, &q);
            mpu.dmpGetLinearAccelInWorld(&aaWorld, &aaReal, &q);
            printf("aworld %6d %6d %6d    ", aaWorld.x, aaWorld.y, aaWorld.z);
        #endif
    
        #ifdef OUTPUT_TEAPOT
            // display quaternion values in InvenSense Teapot demo format:
            teapotPacket[2] = fifoBuffer[0];
            teapotPacket[3] = fifoBuffer[1];
            teapotPacket[4] = fifoBuffer[4];
            teapotPacket[5] = fifoBuffer[5];
            teapotPacket[6] = fifoBuffer[8];
            teapotPacket[7] = fifoBuffer[9];
            teapotPacket[8] = fifoBuffer[12];
            teapotPacket[9] = fifoBuffer[13];
            Serial.write(teapotPacket, 14);
            teapotPacket[11]++; // packetCount, loops at 0xFF on purpose
        #endif
	    
        printf("\n");
    }
}

/************************************* OUR CODE **********************************************/

const char* packetToCStr(int packetNum, float packet[], int length) {
  std::stringstream buf;

  buf << packetNum << " ";
  
  for(int i = 0; i < length; i++) {
    buf << packet[i] << " ";
  }

  const std::string& tmp = buf.str();
  const char* ret = tmp.c_str();
  return ret;
  
}

int readAdc(int *input) {

  unsigned char data[3];
  int a2dVal;
  
  for(int a2dChannel = 0; a2dChannel < 5; a2dChannel++) {
    data[0] = 1; // first byte start bit
    data[1] = 0b10000000 |(((a2dChannel & 7) << 4)); //channel
    data[2] = 0; //don't care
  
    a2d.spiWriteRead(data, sizeof(data));
    
    a2dVal = 0;
    a2dVal = (data[1] << 8) & 0b1100000000;
    a2dVal |= (data[2] & 0xff);

    input[a2dChannel] = a2dVal;
  }
  return 1;
}

/*********************************************** MAIN **********************************************/


int main(int argc, char** argv)
{

  if (argc != 3) {
    cout << "wrong # of args" << endl << "usage: ./udpClient <server_ip_addr> <samplePeriod (s)>" << endl;
    return 0;
  }
  char *ipAddr = argv[1];
  float samplePeriod = atof(argv[2]);
  struct sockaddr_in myaddr, remaddr;
  int fd, slen=sizeof(remaddr);
  char buf[BUFLEN];	/* message buffer */
  int recvlen;		/* # bytes in acknowledgement message */

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

  /* now let's send the messages */

  int i = 0;
  int adcVals[5] = {0};
  float bicepYpr[3] = {0};

  // set up dmp
  dmpSetup();
  /*  while(1) {
    dmpLoop(bicepYpr);
    sleep(1);
  }
  */

  while(1) {
    readAdc(adcVals); // poll flex sensors for finger data
    dmpLoop(bicepYpr);
    printf("Sending packet %d to %s port %d\n", i, ipAddr, SERVICE_PORT);
    
    //package up data into character array and send it
    sprintf(buf, "%d %d %d %d %d %d %f %f %f", i++,
	    adcVals[0], adcVals[1], adcVals[2], adcVals[3], adcVals[4],
	    bicepYpr[0], bicepYpr[1], bicepYpr[2]);
    if (sendto(fd, buf, strlen(buf), 0, (struct sockaddr *)&remaddr, slen)==-1) {
      perror("sendto");
      exit(1);
    }
    
    //now receive an acknowledgement from the server 
    recvlen = recvfrom(fd, buf, BUFLEN, 0, (struct sockaddr *)&remaddr, (socklen_t *)&slen);
    if (recvlen >= 0) {
    buf[recvlen] = 0;	// expect a printable string - terminate it 
      printf("received message: \"%s\"\n", buf);
    }
    sleep(samplePeriod);
  }
  return 0;
}
