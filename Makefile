include config.mk

SRC = clparser.c process.c
OBJ = ${SRC:.c=.o}

clparser : ${OBJ}
	$(CC) -o $@ ${OBJ}

.c.o:
	${CC} -c $<

${OBJ}: process.h config.mk

PHONY : clean install

clean :
	rm -f ${OBJ} clparser
	
install : clparser
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f clparser ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/clparser
