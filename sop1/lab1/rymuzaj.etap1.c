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
#define MAXFD 20
#define ERR(source) (perror(source),\
		fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
		exit(EXIT_FAILURE))
		
		
int walk (const char * name, const struct stat*s, int type, struct FTW *f)
{
	printf("%s\n",name);
	
	return 0;
}

int main (int argc, char** argv)
{
	int c;
	
	while ((c=getopt(argc,argv,"p:"))!=-1)
		switch(c)
		{
			case 'p':
				if(nftw(optarg,walk,MAXFD,FTW_PHYS)!=0)
					printf("%s: brak dostepu\n",optarg);
				break;
				case '?':
				default:
					ERR(argv[0]);
		}
	
	return EXIT_SUCCESS;
}
