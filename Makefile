testparse : testparse.o process.o
	cc process.o testparse.o -o testparse

testparse.o : testparse.c process.h
	cc -c testparse.c

process.o : process.h process.c
	cc -c process.c

PHONY : clean

clean :
	rm -f testparse.o process.o testparse
	
