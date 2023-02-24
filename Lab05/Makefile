CFLAGS 		= 	-c -Wall -w
CLIENT_OBJS =	tcp_client.o
SERVER_OBJS =	tcp_server.o llist.o
LIB_OBJS 	= 	llist.o
TARGETS 	=	tcp_client tcp_server

all: ${TARGETS}

clean:
	rm -f ${CLIENT_OBJS} ${SERVER_OBJS} ${TARGETS}

client: 
	./tcp_client 127.0.0.1 5500

server: 
	./tcp_server 5500

llist: ${LIB_OBJS}
	${CC} -o $@ ${LIB_OBJS}

tcp_client: ${CLIENT_OBJS}
	${CC} -o $@ ${CLIENT_OBJS}

tcp_server: ${SERVER_OBJS}
	${CC} -o $@ ${SERVER_OBJS}

tcp_client.o: tcp_client.c
tcp_server.o: tcp_server.c
llist.o:	  llist.c