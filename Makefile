CC = gcc
CFLAGS = -I unpv13e/lib -g -O2 -D_REENTRANT -Wall
LIBS = unpv13e/libunp.a -lpthread

server: server.o
	${CC} ${CFLAGS} -o $@ server.o ${LIBS}

clients: clients.o
	${CC} ${CFLAGS} -o $@ clients.o ${LIBS}