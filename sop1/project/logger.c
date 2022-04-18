#include "logger.h"

// wątek odpowiedzialny za zapis błędów
void* logger(void* args)
{	
	thrData_t* data = (thrData_t*) args;
	int signo, out;
	if (sigwait(data->pMask, &signo)) ERR("sigwait");

	if((out=open(data->consts->name,O_WRONLY|O_CREAT|O_TRUNC|O_APPEND,0777))<0)ERR("open");
	
	while(true)
	{
		if(waitForFileChecker(data)) 
			break;
			
		writeNextLog(data, out);
	}
	
	if(close(out))ERR("close");
	return NULL;
}

// czekanie na dopisanie nowych bledow do kolejki (true gdy zakonczono czytanie)
bool waitForFileChecker (thrData_t* data)
{
	struct timespec t = {0, WAIT_TIME_NS};
	while(data->qHead == NULL)
	{
		if(data->checkingFinished) 
			return true;
		
		if(nanosleep(&t,NULL)) ERR("nanosleep");
	}
	
	return false;
}

// dopisania do pliku kolejnego bledu
void writeNextLog(thrData_t* data, int out)
{
	char msg [LOG_MAX];
	pthread_mutex_lock(data->pmxHead);
	int code;
	char* psrc;
	code = popErr(&data->qHead, &psrc);
	if (code == -1) ERR("pop");
	pthread_mutex_unlock(data->pmxHead);
		
	err2msg (psrc, code, msg);
	free(psrc);
	if(write(out,msg,strlen(msg))<0) ERR("write");
}

// zamiana kodu bledu na wiadomosc
void err2msg (char* src, errCode_t code, char* log)
{
	sourceAndDate(log, src);
	
	switch(code)
	{
		case EMPTY_ERR:
			strcat(log,"pusty folder\n");
			break;
		case SOURCE_ERR:
			strcat(log,"brak pliku zrodlowego\n");
			break;
		case MAKEFILE_ERR:
			strcat(log,"brak makefile\n");
			break;
		case DECL_ERR:
			strcat(log,"brak oswiadczenia\n");
			break;
	}
}

void sourceAndDate(char* log, char* src)
{
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	if (snprintf(log, LOG_MAX, "[%d.%d.%d_%d:%d] (%s) ", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, src)<0) ERR("snprintf");
}
