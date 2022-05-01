CC=gcc

CFLAGS=-g -Wall

all: quic

quic: directory.o main.o
	$(CC) directory.o file.o main.o -o quic

main.o: main.c directory.h directory.c
	$(CC) $(CFLAGS) -c main.c directory.c

directory.o: file.c file.h directory.c directory.h
	$(CC) $(CFLAGS) -c directory.c file.c


clean:
	rm -f *.o quic
