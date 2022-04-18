CC = gcc
LFLAGS = -lrt
CFLAGS = -std=gnu99 -Wall
TARGET = rymuzaj

all: ${TARGET}
${TARGET}: ${TARGET}.c
	${CC} ${CFLAGS} -o ${TARGET} ${TARGET}.c ${LFLAGS}
.PHONY: clean all
clean:
	rm ${TARGET}
