CC=g++

TARGETS=client server

FLAGS=-std=c++11

LIBS=-pthread

all:$(TARGETS)

client: client.cpp MPU/I2Cdev.cpp MPU/MPU6050.cpp RaspberryPi-mcp3008Spi/mcp3008Spi.cpp CommandPacket.cpp 
	$(CC) $(FLAGS) -o $@ $^ -lm

server: threadedserver.cpp CommandPacket.cpp
	$(CC) $(FLAGS) $(LIBS) -o $@ $^

tester: test/test.cpp CommandPacket.cpp
	$(CC) $(FLAGS) $(LIBS) -o $@ $^

.PHONY: clean

clean:
	rm client server test *~
