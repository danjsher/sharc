CC=g++

TARGETS=client server

FLAGS=-std=c++11

LIBS=-pthread

all:$(TARGETS)

client: client.cpp
	$(CC) $(FLAGS) -o $@ $^

server: threadedserver.cpp
	$(CC) $(FLAGS) $(LIBS) -o $@ $^

.PHONY: clean

clean:
	rm client server
