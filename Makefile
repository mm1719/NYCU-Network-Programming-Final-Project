CC = gcc
CFLAGS = -I unpv13e/lib -g -O2 -D_REENTRANT -Wall
LIBS = unpv13e/libunp.a -lpthread

server:	server.o
	${CC} ${CFLAGS} -std=c99 -o $@ server.o ${LIBS}

client:	client.o
	${CC} ${CFLAGS} -o $@ client.o ${LIBS}

test:	test.o
	${CC} ${CFLAGS} -o $@ test.o ${LIBS}