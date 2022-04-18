// Oświadczam, że niniejsza praca stanowiąca podstawę do uznania osiągnięcia efektów uczenia się z przedmiotu SOP2 została wykonana przeze mnie samodzielnie.
// Jakub Rymuza
// 305870

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <signal.h>
#include <netdb.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define ERR(source) (perror(source),                                 \
                     fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), \
                     exit(EXIT_FAILURE))

#define FILE_NAME "shared.txt"
#define STR_LENGTH 64
#define TAB_SIZE 10

void usage()
{
    fprintf(stderr, "Invalid arguments\n");
}

void producer_mode();
void consument_mode();

int main(int argc, char **argv)
{
    if (argc != 2)
        usage();

    char c;
    while ((c = getopt(argc, argv, "p::k::")) != -1)
        switch (c)
        {
        case 'p':
            producer_mode();
            break;
        case 'k':
            consument_mode();
            break;
        case '?':
        default:
            usage();
        }

    if (argc > optind)
        usage();

    return EXIT_SUCCESS;
}

void producer_mode()
{
    int fd;
    if ((fd = shm_open(FILE_NAME, O_RDWR | O_CREAT, 0777)) < 0)
        ERR("open");

    int map_size = STR_LENGTH * TAB_SIZE;

    if(ftruncate(fd,map_size)<0)
        ERR("truncate");

    char *map = mmap(0, map_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(map == (void*)-1)
        ERR("mmap");

    srand(time(NULL));

    for (int i = 0; i < TAB_SIZE; ++i)
    {
        for (int j = 0; j < STR_LENGTH - 1; ++j)
        {
            map[i * STR_LENGTH + j] = rand() % ('z' - 'a') + 'a';
        }
        map[(i + 1) * STR_LENGTH - 1] = '\0';
    }
    
    if(munmap(map, map_size)<0)
        ERR("unmap");
    if (close(fd))
        ERR("close");
}

void consument_mode()
{
    int fd;
    if ((fd = shm_open(FILE_NAME, O_RDWR, 0777)) < 0)
        ERR("open");

    int map_size = STR_LENGTH * TAB_SIZE;
    if(ftruncate(fd,map_size)<0)
        ERR("truncate");

    char *map = mmap(0, map_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if(map == (void*)-1)
        ERR("mmap");

    for (int i = 0; i < TAB_SIZE; ++i)
    {
        printf("%s\n", map + (i * STR_LENGTH));
    }

    if(munmap(map, map_size)<0)
        ERR("unmap");
    if (close(fd))
        ERR("close");
}