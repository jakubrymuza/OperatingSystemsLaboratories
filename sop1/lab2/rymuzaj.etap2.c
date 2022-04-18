// Oświadczam, że niniejsza praca stanowiąca podstawę do uznania osiągnięcia efektówuczenia się z przedmiotu SOP1 została wykonana przeze mnie samodzielnie.
// Jakub Rymuza
// 305870


#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#define ERR(source) (fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
                     perror(source),kill(0,SIGKILL),\
		     		     exit(EXIT_FAILURE))
		     		     
#define IN_SIZE 20
#define PATH_MAX 100
#define MAX_CODE (IN_SIZE*sizeof(char))

volatile sig_atomic_t last_signal = 0;

void sethandler( void (*f)(int), int sigNo)
{
    struct sigaction act;
    memset(&act, 0, sizeof(struct sigaction));
    act.sa_handler = f;
	if (-1==sigaction(sigNo, &act, NULL)) ERR("sigaction");
}

void sig_handler(int sig)
{
    last_signal = sig;
}

void send_signals(int signal,int count, int k, pid_t pid)
{
	struct timespec slp = {0, k*1000};
	while(count>0)
	{
		nanosleep(&slp, NULL);
		if(kill(pid, signal)<0) ERR("kill");
		--count;
	}
}

int receive_signals(int t)
{
	int count = 0;
	
	while(sleep(t))
	{
		if (last_signal == SIGUSR1)
			++count;
		else ERR("wrong signal");
	}
	
	return count;
}

int wait_first()
{
	last_signal = 0;
	sigset_t mask;
	sigemptyset(&mask);
	sigsuspend (&mask);
	return last_signal;
}

int receiving(int t)
{
	int sigcount = 0;
	
	pause();
	int first = last_signal;
	
	if (first == SIGUSR1)
		++sigcount;
	else ERR("wrong signal"); 
	
	sigcount += receive_signals(t);
	
	return sigcount;
}

void last_child(int t, char* path)
{
	int sigcount = receiving(t);
	printf("Last number of signals: %d\n", sigcount);
}

void child (int n, int k, int t, char* path)
{
	pid_t pid;
	if((pid = fork())<0) ERR("fork");
	if (!pid)
	{
		if (n == 1)
		{
			last_child(t,path);
			return;
		} 
		else
		{
			child(--n, k, t, path);
			return;
		}
	}
	
	int sigcount = receiving(t);
	
	send_signals(SIGUSR1, sigcount, k, pid);
	
	printf("Number of signals: %d\n", sigcount);
}


int main (int argc, char** argv)
{
	
	if (argc != 5) ERR("invalid arg number");
	int n = atoi(argv[1]), k, t;
	if (n<2 || n>10) ERR("invalid n value");
	k = atoi(argv[2]);
	if (k<1 || k>1000) ERR("invalid k value");
	t = atoi(argv[3]);
	
	char* path = malloc(PATH_MAX*sizeof(char));
	strcpy(path,argv[4]);
	
	sethandler(sig_handler,SIGUSR1);
	sethandler(sig_handler,SIGUSR2);
	
	pid_t pid; 
	if((pid = fork())<0) ERR("fork");
	if (!pid)
	{
		child(--n,k,t,path);
		return EXIT_SUCCESS;
	} 
	
	
	char* buf = malloc((IN_SIZE+1)*sizeof(char));
	
	while(1)
	{
		if(scanf("%s", buf)<0) ERR("readline");
		
		if (strcmp(buf,"exit")==0)
			kill(0,SIGINT);

		int len = strlen(buf);
		
		send_signals(SIGUSR1, len, k, pid);
	}

	
	return EXIT_SUCCESS;
}
