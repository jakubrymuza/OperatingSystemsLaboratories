CC=gcc
LDLIBS= -lrt -lpthread
CFLAGS= -std=gnu99 -Wall
TARGET = generator
TARGET2 = processor

all: ${TARGET} ${TARGET2}

${TARGET}: ${TARGET}.c
	${CC} ${CFLAGS} -o ${TARGET} ${TARGET}.c ${LDLIBS}
	
${TARGET2}: ${TARGET2}.c
	${CC} ${CFLAGS} -o ${TARGET2} ${TARGET2}.c ${LDLIBS}

clean:
	-rm -f ${TARGET} ${TARGET2}

