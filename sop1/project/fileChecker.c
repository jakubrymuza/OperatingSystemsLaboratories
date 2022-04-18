#include "fileChecker.h"

// wątek sprawdzający pliki
void* fileChecker(void* args)
{
	thrData_t* data = (thrData_t*) args;
	struct dirent *dp;
	struct stat filestat;
	DIR* dirp;
	int signo;
	
	if (sigwait(data->pMask, &signo)) ERR("sigwait");

	if((dirp = opendir(".")) == NULL) ERR("opendir");
	do 
	{
		errno = 0;
		if ((dp = readdir(dirp)) != NULL) 
		{
			if (*dp->d_name == '.') continue;
			if (lstat(dp->d_name, &filestat)) ERR("lstat");
			if (S_ISDIR(filestat.st_mode))
				scan_dir(data, dp->d_name);
		}
	} while (dp != NULL);
	
	if (errno != 0) ERR("readdir");
	if(closedir(dirp)) ERR("closedir");
	
	data->checkingFinished = true;
	return NULL;
}

// przeszukanie folderu z rozwiazaniem
void scan_dir(thrData_t* data, char* dirName)
{
	struct dirent *dp = NULL;
	errFlags_t flags = {false, false, true, true};
	
	DIR* dirp;
	
	if((dirp = opendir(dirName)) == NULL) ERR("opendir");
	do {
		errno = 0;
		if ((dp = readdir(dirp)) != NULL)
		 {
			if (*dp->d_name == '.') continue;
			
			setFlags(&flags, dp->d_name, dirName, data->consts->decl);
		}
	} while (dp != NULL);
	if (errno != 0) ERR("readdir");
	if(closedir(dirp)) ERR("closedir");
	
	if (!data->consts->check_makefile)
		flags.makefile = true;
	
	sendAllErrors(&flags, data->pmxHead, &data->qHead, dirName);
}

void setFlags (errFlags_t* flags, char* name, char* dirName, char* decl)
{
	flags->empty = false;
	if(strstr(name,".c")!=NULL)
	{	
		flags->source = true;
				
		if(!findPhrase(dirName, name, decl))
			flags->decl = false;
	}
	else if(strcmp(name,"Makefile")== 0 || strcmp(name,"makefile")== 0)
		flags->makefile = true;
}

void sendAllErrors (errFlags_t* flags, pthread_mutex_t* pmxHead, errNode_t** qHead, char* dirName)
{
	if (flags->empty) 
		sendError(dirName, EMPTY_ERR, pmxHead, qHead);
	else
	{
		if(!flags->source) 
			sendError(dirName, SOURCE_ERR, pmxHead, qHead);
		else if (!flags->decl) 
			sendError(dirName, DECL_ERR, pmxHead, qHead);
		
		if(!flags->makefile) 
			sendError(dirName, MAKEFILE_ERR, pmxHead, qHead);
	}
}  

// funkcja wpisuje blad do kolejki
void sendError(char* src, errCode_t code, pthread_mutex_t* pmx, errNode_t** dest)
{
	pthread_mutex_lock(pmx);
	char* temp = malloc(PATH_MAX*sizeof(char));
	if (temp == NULL) ERR("malloc");
	strcpy(temp,src);
	push(dest, temp, code);
	pthread_mutex_unlock(pmx);
}

// szukanie frazy w pliku
bool findPhrase (char* dirName, char* fileName, char* pattern)
{
	char pathName [PATH_MAX];
	if (snprintf(pathName, PATH_MAX, "./%s/%s", dirName, fileName)<0) ERR("snprintf");
	bool found = false;
	
	int in;
	char buff [READ_BUFF_SIZE];
	
	if((in=open(pathName,O_RDONLY))<0)ERR("open");
	
	int ret = 1;
	while (ret!=0)
	{
		ret = read(in, buff, READ_BUFF_SIZE);
		
		if (ret<0) ERR("read");
		if (strstr(buff, pattern)!= NULL) 
		{
			found = true;
			break;
		}
	}	
	
	if(close(in))ERR("close");
	return found;
}
