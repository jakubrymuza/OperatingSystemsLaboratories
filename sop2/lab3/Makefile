CC=gcc
LDLIBS= -lrt -lpthread
CFLAGS= -std=gnu99 -Wall
TARGET1 = server
TARGET2 = client
UTILS = utils

all: ${TARGET1} ${TARGET2}

${TARGET1}: ${TARGET1}.o ${UTILS}.o
	${CC} -o ${TARGET1} ${TARGET1}.o ${UTILS}.o ${LDLIBS}
	
${TARGET1}.o: ${TARGET1}.c ${UTILS}.h
	${CC} ${CFLAGS} -o ${TARGET1}.o -c ${TARGET1}.c 
	
${TARGET2}: ${TARGET2}.o ${UTILS}.o
	${CC} -o ${TARGET2} ${TARGET2}.o ${UTILS}.o ${LDLIBS}
	
${TARGET2}.o: ${TARGET2}.c ${UTILS}.h
	${CC} ${CFLAGS} -o ${TARGET2}.o -c ${TARGET2}.c 

${UTILS}.o: ${UTILS}.c ${UTILS}.h
	${CC} ${CFLAGS} -o ${UTILS}.o -c ${UTILS}.c 

.PHONY: clean all

clean:
	-rm -f ${TARGET1} ${TARGET2} ${TARGET1}.o ${TARGET2}.o ${UTILS}.o

