#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>

#define DECL_MAX 100
#define SERVER_MAX PATH_MAX+NAME_MAX

#define ERR(source) (perror(source),\
                     fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
                     exit(EXIT_FAILURE))

typedef enum errorCodes
{
	EMPTY_ERR, SOURCE_ERR, MAKEFILE_ERR, DECL_ERR
} errCode_t; 

typedef struct errNode
{
	char* errSrc;
	errCode_t errCode;
	struct errNode* next;
} errNode_t;

typedef struct inputArguments
{
	bool check_makefile;
	char decl[DECL_MAX];
	char server[SERVER_MAX];
	char path[PATH_MAX];
	char name[NAME_MAX];	
} inArgs_t;

typedef struct threadData
{
	inArgs_t* consts;
	pthread_t loggerTid;
	pthread_t fileCheckerTid;
	
	errNode_t* qHead;
	pthread_mutex_t* pmxHead;
	
	bool checkingFinished;	
	sigset_t* pMask;
} thrData_t;

int popErr(errNode_t**, char**);
void push(errNode_t**, char*, errCode_t);

#endif
