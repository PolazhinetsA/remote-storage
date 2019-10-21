all: server client

server: server.c sendrecv.c userbase.c sendrecv.h userbase.h commondef.h
	gcc -o server server.c sendrecv.c userbase.c -lpthread

client: client.c sendrecv.c sendrecv.h commondef.h
	gcc -o client client.c sendrecv.c

