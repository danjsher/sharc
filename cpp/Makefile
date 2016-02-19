
CC=g++

TARGETS=client server udpClient udpServer

FLAGS=-std=c++11

LIBS=-pthread

all:$(TARGETS)

client: client.cpp MPU/I2Cdev.cpp MPU/MPU6050.cpp RaspberryPi-mcp3008Spi/mcp3008Spi.cpp
	$(CC) $(FLAGS) -o $@ $^ -lm

server: threadedserver.cpp 
	$(CC) $(FLAGS) $(LIBS) -o $@ $^

udpClient: udpClient.cpp MPU/I2Cdev.cpp MPU/MPU6050.cpp RaspberryPi-mcp3008Spi/mcp3008Spi.cpp 
	$(CC) $(FLAGS) -o $@ $^ -lm

udpServer: udpServer.cpp 
	$(CC) $(FLAGS) $(LIBS) -o $@ $^

tester: test/test.cpp 
	$(CC) $(FLAGS) $(LIBS) -o $@ $^

.PHONY: clean

clean:
	rm $(TARGETS) *~
