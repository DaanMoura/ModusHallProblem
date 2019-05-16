CC=gcc -Wall

all: 
	$(CC) trabalho.c -o trabalho.o -pthread
	clear
	./trabalho.o

warnings:
	$(CC) trabalho.c -o trabalho.o -pthread
