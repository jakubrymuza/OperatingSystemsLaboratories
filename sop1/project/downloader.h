#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include "definitions.h"
#define COMMAND_MAX SERVER_MAX+20

void downloader(thrData_t*);
void download (char*);
bool isArchive (char*);
void unpackAll();
void unpack (char*);

#endif
