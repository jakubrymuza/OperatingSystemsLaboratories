// Oświadczam, że niniejsza praca stanowiąca podstawę do uznania osiągnięcia efektów uczenia się z przedmiotu SOP2 została wykonana przeze mnie samodzielnie.
// Jakub Rymuza
// 305870

#include "utils.h"
#define STDIN_FD 0
#define NOT_NOW_MSG "nie teraz"
#define NO_ANSWER_MSG "0"
volatile sig_atomic_t last_signal = 0;

typedef struct server
{
    int fd;
    int loaded;
    int serv_closed;
    char buf[MAX_MSG_LENGTH];

} server_t;

void sighandler(int);
void client_work(server_t *, int, int);
void send_answer(server_t *);
int read_fragment(server_t *);
void clear_stdin();
int check_msg(char *, char *, int *, int);
void load_questions(int *, fd_set, int, server_t *, server_t **);
void send_no_answer(server_t *);
int all_serv_closed(server_t *, int);

int main(int argc, char **argv)
{
    if (argc % 2 == 0)
        usage();

    if (sethandler(sighandler, SIGINT))
        ERR("sethandler");
    if (sethandler(SIG_IGN, SIGPIPE))
        ERR("sethandler");

    int servers_count = argc / 2, maxfd = 1;
    server_t *servers = malloc(servers_count * sizeof(server_t));
    if (servers == NULL)
        ERR("malloc");

    for (int i = 1, j = 0; j < servers_count; i += 2, j++)
    {
        servers[j].fd = connect_socket(argv[i], argv[i + 1]);
        if (maxfd <= servers[j].fd)
            maxfd = servers[j].fd + 1;

        servers[j].loaded = 0;
        servers[j].serv_closed = 0;
    }

    client_work(servers, servers_count, maxfd);

    for (int i = 0; i < servers_count; ++i)
        if (TEMP_FAILURE_RETRY(close(servers[i].fd)) < 0)
            ERR("close");

    free(servers);
    return EXIT_SUCCESS;
}

void client_work(server_t *servers, int servers_count, int maxfd)
{
    fd_set base_rfds, rfds;
    FD_ZERO(&base_rfds);

    for (int i = 0; i < servers_count; ++i)
        FD_SET(servers[i].fd, &base_rfds);

    FD_SET(STDIN_FD, &base_rfds);

    int question_loaded = 0;
    server_t *ploaded = NULL;

    while (last_signal != SIGINT && !all_serv_closed(servers, servers_count))
    {
        rfds = base_rfds;

        if (select(maxfd, &rfds, NULL, NULL, NULL) < 0)
        {
            if (EINTR == errno)
                continue;
            ERR("pselect");
        }

        if (FD_ISSET(STDIN_FD, &rfds))
        {
            if (!question_loaded)
            {
                clear_stdin();
                printf("%s\n", NOT_NOW_MSG);
            }
            else
            {
                send_answer(ploaded);
                question_loaded = 0;
                ploaded->loaded = 0;
                ploaded = NULL;
            }
        }
        else
            load_questions(&question_loaded, rfds, servers_count, servers, &ploaded);
    }
}

int read_fragment(server_t *pserver)
{
    if(pserver->serv_closed)
        return 0;

    ssize_t size;
    if ((size = TEMP_FAILURE_RETRY(read(pserver->fd, pserver->buf + pserver->loaded, MAX_MSG_LENGTH))) < 0)
    {
        if (errno == EPIPE)
            pserver->serv_closed = 1;
        else
            ERR("read");
    }

    if (check_msg(pserver->buf, END_MSG, &(pserver->serv_closed), pserver->loaded) || check_msg(pserver->buf, TOO_MANY_USERS_MSG, &(pserver->serv_closed), pserver->loaded))
        return 0;

    pserver->loaded += (int)size;

    if (pserver->buf[pserver->loaded - 1] == '\0')
        return 1;

    return 0;
}

void send_answer(server_t *ploaded)
{
    size_t size = 200;
    char *buf = malloc(size * sizeof(char));
    if (buf == NULL)
        ERR("malloc");

    getline(&buf, &size, stdin);

    if (write(ploaded->fd, &buf[0], 1) < 0)
    {
        if (errno == EPIPE)
            ploaded->serv_closed = 1;
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

int all_serv_closed(server_t *servers, int servers_count)
{
    for (int i = 0; i < servers_count; ++i)
        if (!servers[i].serv_closed)
            return 0;

    return 1;
}

void load_questions(int *pquestion_loaded, fd_set rfds, int servers_count, server_t *servers, server_t **ploaded)
{
    for (int i = 0; i < servers_count; ++i)
    {
        if (FD_ISSET(servers[i].fd, &rfds) && *ploaded != &(servers[i]))
        {
            int l = read_fragment(&servers[i]);
            if (l)
                *pquestion_loaded = 1;

            if (l)
            {
                printf("%s\n", servers[i].buf);
                if (*ploaded != NULL)
                    send_no_answer(*ploaded);
                *ploaded = &(servers[i]);
            }
        }
    }
}

void send_no_answer(server_t *old_ploaded)
{
    old_ploaded->loaded = 0;
    if (write(old_ploaded->fd, NO_ANSWER_MSG, sizeof(NO_ANSWER_MSG) - 1) < 0)
    {
        if (errno == EPIPE)
            old_ploaded->serv_closed = 1;
        else
            ERR("write");
    }
}