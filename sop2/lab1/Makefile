CC = gcc
CFLAGS = -std=gnu99 -Wall
TARGET = rymuzaj

all: ${TARGET}
${TARGET}: ${TARGET}.c
	${CC} ${CFLAGS} -o ${TARGET} ${TARGET}.c
.PHONY: clean all
clean:
	rm ${TARGET}
