CFLAGS 		= 	-Wall -ggdb3 -pthread
CC 			= 	gcc
CLIENT_OBJS =	client-socket.o
SERVER_OBJS =	server-socket.o
TARGETS 	=	client server

all: ${TARGETS}

clean:
	rm -f ${CLIENT_OBJS} ${SERVER_OBJS} ${TARGETS}

run-client: 
	./client 127.0.0.1 5550

run-server: 
	./server 5550

val-server: 
	valgrind --tool=memcheck --leak-check=yes ./server 5550

val-client:
	valgrind --tool=memcheck --leak-check=yes ./client 127.0.0.1 5550

client: ${CLIENT_OBJS}
	${CC} ${CFLAGS} -o $@ ${CLIENT_OBJS}

server: ${SERVER_OBJS}
	${CC} ${CFLAGS} -o $@ ${SERVER_OBJS}

client-socket.o: client-socket.c
server-socket.o: server-socket.c
