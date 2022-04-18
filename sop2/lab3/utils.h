// Oświadczam, że niniejsza praca stanowiąca podstawę do uznania osiągnięcia efektów uczenia się z przedmiotu SOP2 została wykonana przeze mnie samodzielnie.
// Jakub Rymuza
// 305870

#ifndef UTILS_H
#define UTILS_H

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <signal.h>
#include <netdb.h>
#include <fcntl.h>
#include <time.h>
#define ERR(source) (perror(source),                                 \
                     fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), \
                     exit(EXIT_FAILURE))

#define MIN(a,b) (((a)<(b))?(a):(b))
#define BACKLOG 3
#define MAX_MSG_LENGTH 2000
#define END_MSG "KONIEC"
#define TOO_MANY_USERS_MSG "NIE"

void usage();
int sethandler(void (*f)(int), int sigNo);
ssize_t bulk_write(int fd, char *buf, size_t count);
ssize_t bulk_read(int fd, char *buf, size_t count);
int add_new_client(int sfd);
int make_socket(int domain, int type);
int bind_tcp_socket(struct sockaddr_in addr);
int make_socket(int domain, int type);
int bind_tcp_socket(struct sockaddr_in addr);
struct sockaddr_in make_address(char *address, char *port);
int connect_socket(char *, char *);

#endif