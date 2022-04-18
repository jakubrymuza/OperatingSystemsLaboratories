#ifndef LOGGER_H
#define LOGGER_H

#include "definitions.h"
#define WAIT_TIME_NS 10000
#define LOG_MAX NAME_MAX+50

void* logger(void*);
void writeNextLog(thrData_t*, int);
void err2msg (char*, errCode_t, char*);
void sourceAndDate(char*, char*);
bool waitForFileChecker (thrData_t*);

#endif
