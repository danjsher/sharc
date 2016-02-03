CC=g++

TARGETS=client server

all:$(TARGETS)

client: client.cpp
	$(CC) -o $@ $^

server: threadedserver.cpp
	$(CC) -o $@ $^

.PHONY: clean

clean:
	rm client server
