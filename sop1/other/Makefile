CC = gcc
CFLAGS = -std=gnu99 -Wall
TARGET = prog1

${TARGET}: ${TARGET}.c
	${CC} ${CFLAGS} -o ${TARGET} ${TARGET}.c

.PHONY: all, clean

clean:
	rm ${TARGET}
