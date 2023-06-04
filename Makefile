include config.mk


CFLAGS += -Wall

clparser : process.o map.o

process.o: process.h process.c map.o
map.o: map.h map.c

testmap: map.o

PHONY : clean install

clean :
	rm -f ${OBJ} clparser *.o
	
install : clparser
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f clparser ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/clparser
