// Oświadczam, że niniejsza praca stanowiąca podstawę do uznania osiągnięcia efektów uczenia się z przedmiotu SOP2 została wykonana przeze mnie samodzielnie.
// Jakub Rymuza
// 305870

#include "utils.h"
#define STDIN_FD 0
#define NOT_NOW_MSG "nie teraz"
volatile sig_atomic_t last_signal = 0;

void sighandler(int);
void client_work(int);
void send_answer(int, int *);
int read_fragment(int, char *, int *, int *);
void clear_stdin();
int check_msg(char *, char *, int *, int);

int main(int argc, char **argv)
{
    // obsluga tylko pojedynczego serwera
    if (argc != 3)
        usage();

    if (sethandler(sighandler, SIGINT))
        ERR("sethandler");
    if (sethandler(SIG_IGN, SIGPIPE))
        ERR("sethandler");

    int fd = connect_socket(argv[1], argv[2]);

    client_work(fd);

    printf("end client\n");
    return EXIT_SUCCESS;
}

void client_work(int fd)
{
    fd_set base_rfds, rfds;
    FD_ZERO(&base_rfds);
    FD_SET(fd, &base_rfds);
    FD_SET(STDIN_FD, &base_rfds);

    int question_loaded = 0;
    int bytes_loaded = 0;
    int serv_closed = 0;
    char buf[MAX_MSG_LENGTH];

    while (last_signal != SIGINT && !serv_closed)
    {
        rfds = base_rfds;

        if (select(fd + 1, &rfds, NULL, NULL, NULL) < 0)
        {
            if (EINTR == errno)
                continue;
            ERR("pselect");
        }

        if (!question_loaded && FD_ISSET(fd, &rfds))
            if ((question_loaded = read_fragment(fd, buf, &bytes_loaded, &serv_closed)))
                printf("%s\n", buf);

        if (FD_ISSET(STDIN_FD, &rfds))
        {
            if (!question_loaded)
            {
                clear_stdin();
                printf("%s\n", NOT_NOW_MSG);
            }
            else
            {
                send_answer(fd, &serv_closed);
                question_loaded = 0;
                bytes_loaded = 0;
            }
        }
    }
}

int read_fragment(int fd, char *buf, int *ploaded, int *pserv_closed)
{
    ssize_t size;
    if ((size = TEMP_FAILURE_RETRY(read(fd, buf + *ploaded, MAX_MSG_LENGTH))) < 0)
    {
        if (errno == EPIPE)
            *pserv_closed = 1;
        else
            ERR("read");
    }

    if (check_msg(buf, END_MSG, pserv_closed, *ploaded) || check_msg(buf, TOO_MANY_USERS_MSG, pserv_closed, *ploaded))
        return 0;

    *ploaded += (int)size;

    if (buf[*ploaded - 1] == '\0')
        return 1;

    return 0;
}

void send_answer(int fd, int *pserv_closed)
{
    size_t size = 200;
    char *buf = malloc(size * sizeof(char));
    if (buf == NULL)
        ERR("malloc");

    getline(&buf, &size, stdin);

    if (write(fd, &buf[0], 1) < 0)
    {
        if (errno == EPIPE)
            *pserv_closed = 1;
        else
            ERR("write");
    }

    free(buf);
}

void sighandler(int sig)
{
    last_signal = sig;
}

void clear_stdin()
{
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF)
        ;
}

int check_msg(char *buf, char *msg, int *pserv_closed, int loaded)
{
    if (strncmp(buf + loaded, msg, strlen(msg)) == 0)
    {
        *pserv_closed = 1;
        printf("%s\n", msg);
        return 1;
    }

    return 0;
}