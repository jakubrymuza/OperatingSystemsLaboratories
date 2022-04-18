CC=gcc
CFLAGS = -std=gnu99 -Wall
LDLIBS = -lpthread
NAME = lab3
all: ${NAME}
${NAME}: ${NAME}.c
	${CC} ${CFLAGS} -o ${NAME} ${NAME}.c ${LDLIBS}
.PHONY: clean all
clean:
	rm ${NAME}
