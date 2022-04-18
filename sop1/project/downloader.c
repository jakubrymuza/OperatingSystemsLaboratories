#include "downloader.h"

// wątek ściągający archiwa i rozpakowujący je
void downloader(thrData_t* data)
{	
	download(data->consts->server);
	
	unpackAll();	
	
	pthread_kill(data->fileCheckerTid, SIGUSR1);
	pthread_kill(data->loggerTid, SIGUSR1);
}

void download (char* server)
{
	char command [COMMAND_MAX];
	if(snprintf(command, COMMAND_MAX, "scp %s .", server)<0) ERR("snprintf");
	if (system(command)!=0) ERR("scp");
}

// rozpakowanie plikow
void unpackAll()
{
	struct dirent *dp;
	struct stat filestat;
	DIR* dirp;

	
	if((dirp = opendir(".")) == NULL) ERR("opendir");
	do 
	{
		errno = 0;
		if ((dp = readdir(dirp)) != NULL) 
		{
			if (!isArchive(dp->d_name)) continue;
			
			if (lstat(dp->d_name, &filestat)) ERR("lstat");
			
			if (S_ISREG(filestat.st_mode))
				unpack(dp->d_name);
		}
	} while (dp != NULL);
	if (errno != 0) ERR("readdir");
	if(closedir(dirp)) ERR("closedir");
}

bool isArchive (char* name)
{
	return !(strstr(name,".tar.bz2")==NULL && strstr(name,".tar.gz")==NULL && strstr(name,".tar.xz")==NULL && strstr(name,".zip")==NULL);
}

void unpack (char* name)
{
	char command [COMMAND_MAX];
	if (snprintf(command, COMMAND_MAX, "tar -xjf %s --one-top-level", name)<0) ERR("snprintf");
	if (system(command)!=0) ERR("tar");
}
