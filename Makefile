CC=gcc -Wall

all: 
	$(CC) trabalho.c -o trabalho.o -pthread
	clear
	./trabalho

warnings:
	$(CC) trabalho.c -o trabalho.o -pthread
