// Oświadczam, że niniejsza praca stanowiąca podstawę do uznania osiągnięcia efektów uczenia się z przedmiotu SOP2 została wykonana przeze mnie samodzielnie.
// Jakub Rymuza
// 305870

#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <limits.h>
#include <sys/wait.h>
#include <string.h>

#define FIFO_NAME "graph.fifo"
#define COMMAND_SIZE 8
#define MSG_SIZE 5
#define ALL -2
#define PRINT_BASE 30
#define PRINT_SIZE PRINT_BASE + 50

#define ERR(source) (perror(source),                                 \
                     fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), \
                     exit(EXIT_FAILURE))

volatile sig_atomic_t last_signal = 0;

void usage(char *);

void sethandler(void (*f)(int), int);

void sig_handler(int);

int readMsg(int, int **, int);
void freefd(int **, int);
void open_pipes(int **, int);
void make_fifo();
void vertice_work(int **, int, int);
void close_pipes(int **, int, int, int);
void create_children(int **, int);
void operate_fifo(int **, int);
void parse_buf(int **, char *, int, int);
void print(int *, int, int);
void conn(int **, int *, int, int, char *);

int main(int argc, char **argv)
{
    if (argc != 2)
        usage(argv[0]);
    int n = atoi(argv[1]);

    sethandler(sig_handler, SIGINT);
    sethandler(sig_handler, SIGUSR1);
    sethandler(sig_handler, SIGUSR2);

    int **fd = malloc(n * sizeof(int *));
    if (fd == NULL)
        ERR("malloc");
    for (int i = 0; i < n; ++i)
        if (NULL == (fd[i] = malloc(2 * sizeof(int))))
            ERR("malloc");

    open_pipes(fd, n);
    create_children(fd, n);

    close_pipes(fd, ALL, n, 0);
    operate_fifo(fd, n);

    while (wait(NULL) > 0);

    close_pipes(fd, ALL, n, 1);
    freefd(fd, n);
    return EXIT_SUCCESS;
}

void operate_fifo(int *fd[2], int n)
{
    int fifo;
    make_fifo();

    if ((fifo = open(FIFO_NAME, O_RDONLY)) < 0)
    {
        if (errno == EINTR)
        {
            kill(0, SIGINT);
            return;
        }
        else
            ERR("open");
    }

    while (last_signal != SIGINT && readMsg(fifo, fd, n));

    kill(0, SIGINT);

    if (TEMP_FAILURE_RETRY(close(fifo)) < 0)
        ERR("close");
}

int readMsg(int fifo, int *fd[2], int n)
{
    int count;
    char buf[PIPE_BUF];

    if ((count = read(fifo, buf, PIPE_BUF)) < 0)
    {
        if (errno == EINTR)
        {
            if(last_signal == SIGINT)
                return 0;
        }
        else
            ERR("write");
    }

    if (count > 0)
    {
        buf[count] = '\0';

        parse_buf(fd, buf, count, n);
    }

    memset(buf, 0, count);
    return count;
}

#define P_LEN 6
#define A_LEN 8
#define C_LEN 9

void parse_buf(int *fd[2], char *buf, int count, int n)
{
    char bufmsg[MSG_SIZE];
    int x, y;

    while (count >= P_LEN - 1)
    {
        if (strncmp("print", buf, 5) == 0)
        {
            strcpy(bufmsg, "pri ");
            for (int i = 0; i < n; ++i)
            {
                if (TEMP_FAILURE_RETRY(write(fd[i][1], bufmsg, MSG_SIZE)) == -1)
                    ERR("write");
            }
            buf += P_LEN;
            count -= P_LEN;
        }
        else if (strncmp("add", buf, 3) == 0)
        {
            sscanf(buf, "add %d %d", &x, &y);
            snprintf(bufmsg, MSG_SIZE, "a%c%c ", x, y);

            if (TEMP_FAILURE_RETRY(write(fd[x][1], bufmsg, MSG_SIZE)) == -1)
                ERR("write");

            buf += A_LEN;
            count -= A_LEN;
        }
        else if (strncmp("conn", buf, 4) == 0)
        {
            sscanf(buf, "conn %d %d", &x, &y);
            snprintf(bufmsg, MSG_SIZE, "c%c%c%c", (char)x, (char)y, (char)(3*n));

            if (last_signal == SIGINT) return; 
            else kill(0,SIGUSR2); // "wyzeruj" last_signal we wszystkich procesach

            if (TEMP_FAILURE_RETRY(write(fd[x][1], bufmsg, MSG_SIZE)) == -1)
                ERR("write");
            
            pause(); // czekaj na zakonczenie szukania  
            if (last_signal == SIGINT) break;         

            buf += C_LEN;
            count -= C_LEN;
        }
        else
        {
            printf("niewlasciwa komenda\n");
            break;
        }
    }
}

void create_children(int *fd[2], int n)
{
    int j = 0;

    while (j < n)
    {
        switch (fork())
        {
        case -1:
            ERR("fork");
        case 0:
            vertice_work(fd, j, n);
            exit(EXIT_SUCCESS);
        }
        ++j;
    }
}

void open_pipes(int *fd[2], int n)
{
    for (int i = 0; i < n; ++i)
        if (pipe(fd[i]))
            ERR("pipe");
}

void make_fifo()
{
    if (mkfifo(FIFO_NAME, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP) < 0)
    {
        if (errno != EEXIST)
            ERR("create fifo");

        if (unlink(FIFO_NAME) < 0)
            ERR("remove fifo");

        if (mkfifo(FIFO_NAME, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP) < 0)
            ERR("create fifo");
    }
}

void freefd(int *fd[2], int n)
{
    for (int i = 0; i < n; ++i)
        free(fd[i]);

    free(fd);
}

void vertice_work(int *fd[2], int j, int n)
{
    close_pipes(fd, j, n, 0);
    char buf[MSG_SIZE];
    int count;
    sethandler(sig_handler, SIGINT);
    sethandler(sig_handler, SIGUSR1);
    sethandler(sig_handler, SIGUSR2);

    int *neighbors; // czy istnieje krawedz z wierzcholka j do danego wierzcholka
    if (NULL == (neighbors = malloc(n * sizeof(int))))
        ERR("malloc");

    for (int i = 0; i < n; ++i)
        neighbors[i] = 0;

    while (last_signal != SIGINT)
    {
        if ((count = read(fd[j][0], buf, MSG_SIZE)) == -1)
        {
            if (errno == EINTR)
            {
                if(last_signal == SIGINT)
                {
                    kill(0, SIGINT);
                    return;
                }
            }
            else
                ERR("write");
        }

        buf[count] = '\0';

        if (buf[0] == 'a')
        {
            neighbors[(int)(buf[2])] = 1;
            //printf("dodano krawedz od %d do %d\n", j, (int)buf[2]);
        }
        else if (buf[0] == 'p')
            print(neighbors, n, j);
        else if (buf[0] == 'c')
            conn(fd, neighbors, n, j, buf);
    }

    if (TEMP_FAILURE_RETRY(close(fd[j][0])))
        ERR("close");

    free(neighbors);
    close_pipes(fd, ALL, n, 1);
}

void print(int *neighbors, int n, int j)
{
    char msg [PRINT_SIZE];
    sprintf(msg,"istnieje krawedz od %d do:",j);
    int ncount = 0;
    for (int i = 0; i < n; ++i)
    {
        if (neighbors[i])
        {
            ncount++;
            char temp[5];
            sprintf(temp," %d,",i);
            strcat(msg,temp);
        }
    }
    if(ncount) printf("%s\n",msg);
}


void conn(int *fd[2], int *neighbors, int n, int j, char *buf)
{
    if(last_signal == SIGUSR1 || last_signal == SIGINT) return;

    if ((int)(buf[2]) == j)
    {
        printf("istnieje sciezka od %c do %c\n", (int)(buf[1]+'0'), (int)(buf[2]+'0'));
        kill(0, SIGUSR1); // powiadom o zakonczeniu szukania
        return;
    }
    else
    {
        buf[3] = buf[3] - 1; // zmniejszanie ttl (time to live)
        if ((int)buf[3] == 0)// ttl==0
        {
            printf("nie istnieje sciezka od %c do %c\n", (int)(buf[1]+'0'), (int)(buf[2]+'0'));
            kill(0, SIGUSR1); // powiadom o zakonczeniu szukania
            return;           
        }
        
        int ncount = 0;
        for (int i = 0; i < n; ++i)
        {
            if (neighbors[i])
            {
                ncount++;
                if (TEMP_FAILURE_RETRY(write(fd[i][1], buf, MSG_SIZE)) == -1)
                    ERR("write");
            }
        }

        if (ncount == 0)// brak sasiadow
        {
            if (TEMP_FAILURE_RETRY(write(fd[j][1], buf, MSG_SIZE)) == -1) // napisz do siebie (zeby zginac od ttl)
                ERR("write");
        }
    }
}

void close_pipes(int *fd[2], int j, int n, int rw)
{
    for (int i = 0; i < n; ++i)
    {
        if (i != j || j == ALL)
            if (TEMP_FAILURE_RETRY(close(fd[i][rw])))
                ERR("close");
    }
}

void usage(char *name)
{
    fprintf(stderr, "USAGE: %s\n", name);
    exit(EXIT_FAILURE);
}

void sethandler(void (*f)(int), int sigNo)
{
    struct sigaction act;
    memset(&act, 0, sizeof(struct sigaction));
    act.sa_handler = f;
    if (-1 == sigaction(sigNo, &act, NULL))
        ERR("sigaction");
}

void sig_handler(int sig)
{
    last_signal = sig;
}
