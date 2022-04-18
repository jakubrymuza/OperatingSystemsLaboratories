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

void last_child()
{
	sleep(1);
	printf("%d\n",getpid());
}

void child (int n)
{
	pid_t pid;
	if((pid = fork())<0) ERR("fork");
	if (!pid)
	{
		if (n == 1)
		{
			last_child();
			return;
		} 
		else
		{
			child(--n);
			return;
		}
	}

	
	printf("%d\n",getpid());
	while(wait(NULL)>0);
}


int main (int argc, char** argv)
{
	if (argc != 2) ERR("invalid arg number");
	int n = atoi(argv[1]);
	if (n<2 || n>10) ERR("invalid n value");
	
	pid_t pid; 
	if((pid = fork())<0) ERR("fork");
	if (!pid)
	{
		child(--n);
		return EXIT_SUCCESS;
	} 
	
	
	char* buf = malloc((IN_SIZE+1)*sizeof(char));
	
	while(1)
	{
		if(scanf("%s", buf)<0) ERR("readline");
		
		if (strcmp(buf,"exit")==0)
			kill(0,SIGINT);
			
		
	}
	
	
	while(wait(NULL)>0);
	
	return EXIT_SUCCESS;
}
