CFLAGS = -Wall -g

PORT = 10000

IP_SERVER = 127.0.0.1

CLIENT_ID = user1

all: server subscriber

server: server.cpp

subscriber: subscriber.cpp

.PHONY: clean run_server run_subscriber

run_server:
	./server ${PORT}

run_subscriber:
	./subscriber ${CLIENT_ID} ${IP_SERVER} ${PORT}

clean:
	rm -f server subscriber
