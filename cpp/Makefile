CC=g++

TARGETS=client server

FLAGS=-std=c++11

LIBS=-pthread

all:$(TARGETS)

client: client.cpp MPU/I2Cdev.cpp MPU/MPU6050.cpp RaspberryPi-mcp3008Spi/mcp3008Spi.cpp
	$(CC) $(FLAGS) -o $@ $^ -lm

server: threadedserver.cpp
	$(CC) $(FLAGS) $(LIBS) -o $@ $^

.PHONY: clean

clean:
	rm client server
