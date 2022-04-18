//------------------------------------------------------------------------
// Oświadczam, że niniejsza praca stanowiąca podstawę do uznania
// osiągnięcia efektów uczenia się z przedmiotu SOP została wykonana przeze
// mnie samodzielnie.
// Jakub Rymuza 305870
// ------------------------------------------------------------------------

#include "downloader.h"
#include "fileChecker.h"
#include "logger.h"
#include "definitions.h"

void readArguments (int, char**, inArgs_t*);
void usage();
void defaultValues(inArgs_t*);
void createMainDir (char*);
                     
int main (int argc, char ** argv)
{
	inArgs_t args;
	readArguments(argc, argv, &args);
	thrData_t data;
	data.consts = &args;
	data.checkingFinished = false;
	pthread_mutex_t mxError = PTHREAD_MUTEX_INITIALIZER;
	data.pmxHead = &mxError;
	data.qHead = NULL;
		
	sigset_t newMask;
    sigemptyset(&newMask);
    sigaddset(&newMask, SIGUSR1);
    if (pthread_sigmask(SIG_BLOCK, &newMask, NULL)) ERR("sigmask");
    data.pMask = &newMask;
    
	createMainDir(args.path);
	
	if(pthread_create(&data.fileCheckerTid, NULL, fileChecker, &data) != 0) ERR("create");
	
	if(pthread_create(&data.loggerTid, NULL, logger, &data) != 0) ERR("create");
	
	downloader(&data);
	
	if(pthread_join(data.fileCheckerTid, NULL)!=0) ERR("join");
	if(pthread_join(data.loggerTid, NULL)!=0) ERR("join");
	
	return EXIT_SUCCESS;
}

void usage()
{
	fprintf(stderr, "Invalid argument(s)!\n");
	fprintf(stderr, "USAGE:\n");
	fprintf(stderr, "-m -> check Makefile (optional)\n");
	fprintf(stderr, "-o TEXT ->  declaration phrase (optional)\n");
	fprintf(stderr, "-s SERVER -> server adress (optional)\n");
	fprintf(stderr, "-d PATH -> destination path (optional)\n");
	fprintf(stderr, "-b NAME -> log file name (optional)\n");
	exit(EXIT_FAILURE);
}

void readArguments (int argc, char** argv, inArgs_t* input)
{	
	defaultValues(input);
		
	char c;
	while ((c = getopt (argc, argv, "mo:s:d:b:")) != -1)
		switch (c)
		{
			case 'm':
				input->check_makefile = true;
				break;
			case 'o':
				strcpy(input->decl, optarg);
				break;
			case 's':
				strcpy(input->server, optarg);
				break;
			case 'd':
				strcpy(input->path, optarg);
				break;
			case 'b':
				strcpy(input->name, optarg);
				break;
			case '?':
			default: usage();
		}
		
	if(argc>optind) usage();
}

void defaultValues(inArgs_t* input)
{		
	input->check_makefile = false;
	strcpy(input->decl, "Oswiadczam");
	strcpy(input->server, "nowakj@ssh.mini.pw.edu.pl:/home");
	char cwd [PATH_MAX];
	if(getcwd(cwd, PATH_MAX)==NULL) ERR("getcwd");
	strcpy(input->path, cwd);
	strcpy(input->name, "errors.log");
}

void createMainDir (char* path)
{
	struct stat st = {0};
	if (stat(path, &st) == -1)
		if(mkdir(path, 0700)!=0) ERR("mkdir");

	if(chdir(path)!=0)ERR("chdir");
}
