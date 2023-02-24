CFLAGS = -c -Wall
CC = gcc
LIBS =  -lm 

all: account

account: main.o llist.o
	${CC} main.o llist.o -o account

main.o: main.c
	${CC} ${CFLAGS} main.c

llist.o: llist.c
	${CC} ${CFLAGS} llist.c

clean:
	rm -f *.o *~
