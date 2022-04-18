CC=gcc
C_FLAGS = -std=gnu99 -Wall
L_FLAGS = -lpthread
TARGET = proj_ssh
FILES = main.o downloader.o fileChecker.o logger.o definitions.o
all: ${TARGET}
${TARGET}: ${FILES}
	${CC} -o ${TARGET} ${FILES} ${L_FLAGS}
main.o: main.c downloader.h fileChecker.h logger.h definitions.h
	${CC} -o main.o -c main.c ${C_FLAGS}

downloader.o: downloader.c downloader.h definitions.h
	${CC} -o downloader.o -c downloader.c ${C_FLAGS}

fileChecker.o: fileChecker.c logger.h definitions.h
	${CC} -o fileChecker.o -c fileChecker.c ${C_FLAGS}

logger.o: logger.c logger.h definitions.h
	${CC} -o logger.o -c logger.c ${C_FLAGS}

definitions.o: definitions.c definitions.h
	${CC} -o definitions.o -c definitions.c ${C_FLAGS}

.PHONY: clean all
clean:
	-rm -f ${FILES} ${TARGET}



