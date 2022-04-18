#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define STRLEN 100

#define ERR(source) (perror(source),\
                     fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
                     exit(EXIT_FAILURE))

void usage(char *name)
{
    fprintf(stderr, "USAGE: %s\n", name);
    exit(EXIT_FAILURE);
}

char * mystrcat(char* s1, const char* s2)
{
    int i = 0, j = 0;
    while(*(s1+i) != '\0') ++i;

    while(*(s2+j) != '\0')
    {
        *(s1+i+j) = *(s2+j);
        ++j;
    }

    *(s1+i+j) = '\0';

    return s1;
}

int main(int argc, char **argv)
{
    char* s1;
    int i = 0;
    if (argc != 3)
        usage(argv[0]);

    if (NULL == (s1 = (char*) malloc((strlen(argv[1])+strlen(argv[2]))*sizeof(char)))) ERR("malloc");

    while(*(argv[1]+i) != '\0')
    {
        *(s1+i) = *(argv[1]+i);
        ++i;
    }  

    *(s1+i)='\0';
    
    printf("%s\n", mystrcat(s1, argv[2]));
    free(s1);
    return EXIT_SUCCESS;
}
