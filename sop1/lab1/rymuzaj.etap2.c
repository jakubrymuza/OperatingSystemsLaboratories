// Oswiadczam, ze niniejsza praca stanowiaca podstawe do uznania osiagniecia efektow
// uczenia sie z przedmiotu SOP1 zostala wykonana przez mnie samodzielnie
// Jakub Rymuza
// 305870

#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <ftw.h>
#include <errno.h>
#include <time.h>
#define MAXFD 20
#define ERR(source) (perror(source),\
		fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
		exit(EXIT_FAILURE))
#define MAX_LENGTH 80


char tt;	
FILE* stream;
char patt [MAX_LENGTH];
		
int walk (const char * name, const struct stat*s, int type, struct FTW *f)
{
	char currt;
	switch(type)
	{
		case FTW_D: currt = 'd'; break;
		case FTW_F: currt = 'r'; break;
		case FTW_SL: currt = 's'; break;
		default: break;
	}
	
	char* env = getenv("SIZE");
	//printf("%s\n",patt);
	
	
	if (tt == 'a' || currt == tt)
	{
		if(env)
		{
			struct stat str;
			stat(name,&str);
			long int s = str.st_size;
			if(atoi(env)<s)
				fprintf(stream,"%s\n",name);
		}
		else fprintf(stream,"%s\n",name);
	}

	
	return 0;
}

int main (int argc, char** argv)
{
	int c;
	tt = 'a';
	char name [MAX_LENGTH];
	char temp [MAX_LENGTH];
	char file[MAX_LENGTH];
	srand (time(NULL));
	stream = stdout;
	
	while ((c=getopt(argc,argv,"p:t:s:f:"))!=-1)
		switch(c)
		{
			case 'p':
				strcpy(name,optarg);
				break;
			case 't':
				tt = optarg[0];
				break;
			case 's':
				strcpy(temp,"SIZE=");
				strcat(temp,optarg);
				putenv(temp);
				break;	
			case 'f':
				strcpy(file,optarg);
				int r = rand()%5;
				if(r<3)
					strcat(file,".txt");
				else strcat(file,".out");
				stream=fopen(file,"w+");
				break;
			case '?':
			default:
					ERR(argv[0]);
		}
	
	//while(fgets(patt,MAX_LENGTH,stdin)!=NULL)
	{
		if(nftw(name,walk,MAXFD,FTW_PHYS)!=0)
			printf("%s: brak dostepu\n",optarg);
	}
	
	return EXIT_SUCCESS;
}
