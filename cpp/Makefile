
CC=g++

TARGETS=client server udpClient udpServer

FLAGS=-std=c++11

LIBS=-pthread

all:$(TARGETS)

udpClient: udpClient.cpp MPU/I2Cdev.cpp MPU/MPU6050.cpp RaspberryPi-mcp3008Spi/mcp3008Spi.cpp
	$(CC) $(FLAGS) -o $@ $^ -lm -lwiringPi

udpServer: udpServer.cpp AdafruitPwm.cpp MPU/I2Cdev.cpp
	$(CC) $(FLAGS) $(LIBS) -o $@ $^ -lm -lwiringPi

i2c-test: test/i2c-test.cpp MPU/I2Cdev.cpp AdafruitPwm.cpp
	$(CC) $(FLAGS) $(LIBS) -o $@ $^ -lm -lwiringPi

tester: test/test.cpp 
	$(CC) $(FLAGS) $(LIBS) -o $@ $^

.PHONY: clean

clean:
	rm $(TARGETS) i2c-test *~
