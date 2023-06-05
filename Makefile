include config.mk

# TODO figure out compiling a .so/.a lib so it can be included easier

CFLAGS  += -Wall -Wextra
ARFLAGS += -oU

#clparser : parseargs.o printargs.o map.o
clparser : printargs.o libparseargs.a

(%) : % ;
%.a :
	$(AR) $(ARFLAGS) $@ $?
libparseargs.a:  libparseargs.a(parseargs.o map.o)
#libparseargs.so: parseargs.o map.o

map.o: map.h
parseargs.o: parseargs.h map.h
printargs.o: printargs.h parseargs.h map.h

testmap: map.o

PHONY : clean install

clean :
	rm -f ${OBJ} clparser *.o *.a *.so
	
install : clparser
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f clparser ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/clparser
	mkdir -p ${DESTDIR}${MANPREFIX}/man1
	cp -f clparser.1 ${DESTDIR}${MANPREFIX}/man1/clparser.1
	chmod 644 ${DESTDIR}${MANPREFIX}/man1/clparser.1
