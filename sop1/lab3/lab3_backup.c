//------------------------------------------------------------------------
// Oświadczam, że niniejsza praca stanowiąca podstawę do uznania
// osiągnięcia efektów uczenia się z przedmiotu SOP została wykonana przeze
// mnie samodzielnie.
// Jakub Rymuza 305870
// ------------------------------------------------------------------------

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdarg.h>
#include <pthread.h>
#include <stdbool.h>
#include <limits.h>

#define ERR(source)\
    (perror(source),\
    fprintf(stderr, "%s:%d", __FILE__, __LINE__),\
    exit(EXIT_FAILURE))
    
#define TAB_SIZE 2*n
#define SLEEP_MAX 1000
#define SLEEP_MIN 100
pthread_t* tids;

typedef struct odata
{
	pthread_t tid;
	int t;
} odata_t;


void *overseer(void * args)
{
	sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGQUIT);
    if(pthread_sigmask(SIG_BLOCK, &mask, NULL)!=0) ERR("sigmask");
    
	odata_t* data = (odata_t*)args;
	int s = (data->t)/1000;
	int ms = data->t-s*1000;
	struct timespec rqtp = {s,ms*1000};
	struct timespec rmtp, rqtp2;
	while(1)
	{
		rqtp2 = rqtp;
		while (nanosleep(&rqtp2,&rmtp)!=0)
		{
			rqtp2 = rmtp;
		}
		printf("message\n");
	}
}

void* generator (void* args)
{	
	sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGQUIT);
    if(pthread_sigmask(SIG_BLOCK, &mask, NULL)!=0) ERR("sigmask");
	
	while(1)
	{
		int ms = rand()%(SLEEP_MAX-SLEEP_MIN+1)+SLEEP_MIN;
		struct timespec rqtp = {0,ms*1000};
		nanosleep(&rqtp,NULL);

		printf("generator %ld\n",pthread_self());
	}
}

    
int main (int argc, char** argv)
{
	if (argc != 3) ERR("arguments");
	int n, t;
	n = atoi(argv[1]);
	if (n<1 || n>20) ERR("arguments");
	t = atoi(argv[2]);
	if (t<100 || t>5000) ERR("arguments");
	
	tids = (pthread_t*) malloc (TAB_SIZE*sizeof(pthread_t));
	if (tids == NULL) ERR ("malloc");
	
	odata_t oargs;
	oargs.t = t;
	srand(time(NULL));
	
	if(pthread_create(&(oargs.tid), NULL, overseer, &oargs)!=0) ERR("create");
	
	
	sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGQUIT);
    if(pthread_sigmask(SIG_BLOCK, &mask, NULL)!=0) ERR("sigmask");
    int sig;
    int tab_index = 0;
    
	while (1)
	{
		if(sigwait(&mask, &sig)) ERR("sigwait");
		if(sig == SIGINT) 
		{
			if(tab_index+1 == TAB_SIZE)
			{
				printf("too many generators\n");
				continue;
			}
			if(pthread_create(&(tids[tab_index++]), NULL, generator, NULL)!=0) ERR("create");
		} 
		else if(sig == SIGQUIT) break;
		else ERR("bad signal");

	}
	
	for (int i = 0;i<tab_index; ++i)
	{
		if (pthread_cancel(tids[i])!=0) ERR("cancel");
		if (pthread_join(tids[i],NULL)!=0) ERR("join");
	}
	
	if (pthread_cancel(oargs.tid)!=0) ERR("cancel");
	if(pthread_join(oargs.tid, NULL)!=0) ERR("join");
	
	free(tids);
	
	printf("all good");
	return EXIT_SUCCESS;
}
