#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <cstdlib>
#include <iostream>
#include <math.h>
#include <sstream>

#include "MPU/I2Cdev.h"
#include "MPU/MPU6050_6Axis_MotionApps20.h"
#include "RaspberryPi-mcp3008Spi/mcp3008Spi.h"

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

void loop() {
    // if programming failed, don't try to do anything
    if (!dmpReady) return;
    // get current FIFO count
    fifoCount = mpu.getFIFOCount();

    if (fifoCount == 1024) {
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

using namespace std;

int main(int argc, char **argv) {


  if(argc != 3) {
    cout << "wrong # of args" << endl << "useage: ./client <host_ip_addr>" << endl;
    exit(0);
  }

  float sampleSpeed = atof(argv[2]);
			   
  
  /*
   * Setting up connection to server
   */
  
  int socketHandle = socket(AF_INET, SOCK_STREAM, 0);
  if(socketHandle < 0) {
    cout << "Cannot create socket";
    exit(1);
  }

  struct sockaddr_in clientaddr;
  int clientaddr_len;
  memset(&clientaddr, 0, sizeof(clientaddr));
  char* clientHost = argv[1];
  
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
  
  cout << "Connected to server\n";

  /*
   * Connection completed. 
   */

  float packet[8] = {0};
  const char* txData;

  int adcVals[5] = {0};
  int packetNum = 0;
  
  while(true) {
    readAdc(adcVals);
    
    packet[0] = (float) adcVals[0];
    packet[1] = (float) adcVals[1];
    packet[2] = (float) adcVals[2];
    packet[3] = (float) adcVals[3];
    packet[4] = (float) adcVals[4];
    

    txData = packetToCStr(packetNum++, packet, 8);
    write(socketHandle, txData, strlen(txData));
    cout << packetNum << " | " << packet[0] << " | " << packet[1] << " | " << packet[2] << " | " << packet[3] << " | " << adcVals[4] << " | " << endl;
    sleep(sampleSpeed);
  }
  
  /*
  dmpSetup();
  while(1) {
    loop();
  }
  */
  close(socketHandle);
}
