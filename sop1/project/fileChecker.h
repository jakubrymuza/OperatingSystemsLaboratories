#ifndef FILECHECKER_H
#define FILECHECKER_H

#include "definitions.h"
#define READ_BUFF_SIZE 256

typedef struct errorFlags
{
	bool source;
	bool makefile;
	bool decl;
	bool empty;
} errFlags_t;

void* fileChecker(void*);
void scan_dir(thrData_t*, char*);
void setFlags (errFlags_t*, char*, char*, char*);
bool findPhrase (char *, char*, char*);
void sendAllErrors (errFlags_t*, pthread_mutex_t*, errNode_t**, char*);
void sendError(char*, errCode_t, pthread_mutex_t*, errNode_t**);

#endif
