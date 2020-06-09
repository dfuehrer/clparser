CC=cc
testparse : testparse.o process.o
	$(CC) process.o testparse.o -o testparse

testparse.o : testparse.c process.h
	$(CC) -c testparse.c

process.o : process.h process.c
	$(CC) -c process.c

PHONY : clean

clean :
	rm -f testparse.o process.o testparse
	
