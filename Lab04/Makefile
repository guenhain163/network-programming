CFLAGS = -c -Wall -w
CLIENT_OBJS=	udp_client.o
SERVER_OBJS=	udp_server.o
TARGETS=	udp_client udp_server



all: ${TARGETS}

clean:
	rm -f core *.core ${CLIENT_OBJS} ${SERVER_OBJS} ${TARGETS}

client: all
	./udp_client 127.0.0.1 5500

server: all
	./udp_server 5500

udp_client: ${CLIENT_OBJS}
	${CC} -o $@ ${CLIENT_OBJS}

udp_server: ${SERVER_OBJS}
	${CC} -o $@ ${SERVER_OBJS}

udp_client.o: udp_client.c
udp_server.o: udp_server.c