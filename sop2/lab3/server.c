// Oświadczam, że niniejsza praca stanowiąca podstawę do uznania osiągnięcia efektów uczenia się z przedmiotu SOP2 została wykonana przeze mnie samodzielnie.
// Jakub Rymuza
// 305870

#include "utils.h"
#define WAIT_TIME_NS 330 * 1000
#define ANSWER_SIZE 1
#define NO_MORE_QUESTIONS_MSG "Koniec pytan w pliku!"

volatile sig_atomic_t last_signal = 0;
volatile sig_atomic_t no_more_clients = 0;

typedef struct quiz_client
{
    int isFree;
    char question[MAX_MSG_LENGTH];
    int progress;
    int question_size;
    int client_fd;

} quiz_client_t;

void sighandler(int);
void server_work(int, int, int);
void new_quiz(int *, int, int, int *, quiz_client_t *, fd_set *, int);
void receive_answer();
void send_questions(int, int *, int, quiz_client_t *, fd_set *, fd_set *);
void get_answers(int, quiz_client_t *, fd_set, fd_set *, int, fd_set *, int *);
void get_new_question(quiz_client_t *, int);
int find_free(quiz_client_t *, int);
void delete_client(quiz_client_t *, fd_set *, int *);
void send_ending_msg(quiz_client_t *, int);

int main(int argc, char **argv)
{
    if (argc != 5)
        usage();

    int n = atoi(argv[3]);
    if (n <= 0)
        usage();

    int in;
    if ((in = open(argv[4], O_RDONLY)) < 0)
        ERR("open");

    if (sethandler(sighandler, SIGUSR1))
        ERR("sethandler");
    if (sethandler(sighandler, SIGINT))
        ERR("sethandler");
    if (sethandler(SIG_IGN, SIGPIPE))
        ERR("sethandler");

    srand(time(NULL));

    int fd = bind_tcp_socket(make_address(argv[1], argv[2]));

    server_work(fd, n, in);

    if (TEMP_FAILURE_RETRY(close(fd)) < 0)
        ERR("close");

    if (TEMP_FAILURE_RETRY(close(in)) < 0)
        ERR("close");

    return EXIT_SUCCESS;
}

void server_work(int fd, int max_clients, int in)
{
    quiz_client_t *clients = malloc(max_clients * sizeof(quiz_client_t));
    if (clients == NULL)
        ERR("malloc");

    for (int i = 0; i < max_clients; ++i)
        clients[i].isFree = 1;

    fd_set base_wfds, base_rfds;
    sigset_t mask, oldmask;
    FD_ZERO(&base_rfds);
    FD_ZERO(&base_wfds);
    FD_SET(fd, &base_rfds);
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    sigprocmask(SIG_BLOCK, &mask, &oldmask);

    struct timespec ts;
    ts.tv_sec = 2;
    int clients_count = 0, maxfd = fd + 1;

    while (last_signal != SIGINT)
    {
        fd_set rfds = base_rfds;
        if (no_more_clients)
            FD_CLR(fd, &rfds);

        ts.tv_nsec = WAIT_TIME_NS;

        int fdr = pselect(maxfd, &rfds, NULL, NULL, &ts, &oldmask);
        if (fdr < 0)
        {
            if (EINTR == errno)
                continue;
            ERR("pselect");
        }
        else if (fdr == 0) // timeout
        {
            send_questions(maxfd, &clients_count, max_clients, clients, &base_wfds, &base_rfds);
        }
        else
        {
            if (FD_ISSET(fd, &rfds) && !no_more_clients)
                new_quiz(&clients_count, max_clients, fd, &maxfd, clients, &base_wfds, in);
            get_answers(max_clients, clients, rfds, &base_rfds, in, &base_wfds, &clients_count);
        }

        if (clients_count == 0 && last_signal == SIGUSR1)
            break;
    }

    send_ending_msg(clients, max_clients);
    sigprocmask(SIG_UNBLOCK, &mask, NULL);
    free(clients);
}

void new_quiz(int *clients_count, int max_clients, int fd, int *maxfd, quiz_client_t *clients, fd_set *pbase_wfds, int in)
{
    int cfd;
    if ((cfd = add_new_client(fd)) >= 0)
    {
        if (*clients_count < max_clients)
        {

            if (cfd >= *maxfd)
                *maxfd = cfd + 1;

            int i = find_free(clients, max_clients);

            FD_SET(cfd, pbase_wfds);
            if (cfd >= *maxfd)
                *maxfd = cfd + 1;

            clients[i].isFree = 0;
            clients[i].client_fd = cfd;
            get_new_question(&clients[i], in);

            (*clients_count)++;
        }
        else
        {
            if (bulk_write(cfd, TOO_MANY_USERS_MSG, sizeof(TOO_MANY_USERS_MSG)) < 0 && errno != EPIPE && errno == ECONNRESET)
                ERR("write:");

            if (TEMP_FAILURE_RETRY(close(cfd)) < 0)
                ERR("close");
        }
    }
}

void get_answers(int max_clients, quiz_client_t *clients, fd_set rfds, fd_set *pbase_rfds, int in, fd_set *pbase_wfds, int *pclients_count)
{
    char buf[ANSWER_SIZE + 1];
    ssize_t s;

    for (int i = 0; i < max_clients; ++i)
    {
        if (FD_ISSET(clients[i].client_fd, &rfds))
        {
            if ((s = read(clients[i].client_fd, buf, ANSWER_SIZE)) == ANSWER_SIZE)
            {
                buf[ANSWER_SIZE] = '\0';
                FD_CLR(clients[i].client_fd, pbase_rfds);
                get_new_question(&clients[i], in);
            }
            else if (s < 0)
            {
                if (errno == EPIPE || errno == ECONNRESET)
                    delete_client(&clients[i], pbase_wfds, pclients_count);
                else
                    ERR("read");
            }
        }
    }
}

void send_questions(int maxfd, int *pclients_count, int max_clients, quiz_client_t *clients, fd_set *pbase_wfds, fd_set *pbase_rfds)
{
    fd_set wfds = *pbase_wfds;
    struct timeval tv = {0, 0};
    int c;

    if ((c = select(maxfd, NULL, &wfds, NULL, &tv)) < 0)
    {
        if (errno != EINTR)
            ERR("select");
    }

    if (c == 0)
        return;

    for (int i = 0; i < max_clients; ++i)
    {
        if (FD_ISSET(clients[i].client_fd, &wfds) && !clients[i].isFree && clients[i].progress < clients[i].question_size)
        {
            int written;
            if ((written = bulk_write(clients[i].client_fd, clients[i].question + clients[i].progress, MIN(clients[i].question_size - clients[i].progress,rand() % clients[i].question_size+ 1))) < 0)
            {
                if (errno == EPIPE || errno == ECONNRESET)
                {
                    delete_client(&clients[i], pbase_wfds, pclients_count);
                    continue;
                }
                else
                    ERR("write:");
            }

            clients[i].progress += written;

            if (clients[i].progress == clients[i].question_size)
                FD_SET(clients[i].client_fd, pbase_rfds);
        }
    }
}

void get_new_question(quiz_client_t *client, int in)
{
    client->progress = client->question_size = 0;

    char c = ' ';
    while (c != '\n')
    {
        int r;
        if ((r = read(in, &c, 1)) < 0)
            ERR("read");
        else if (r == 0)
        {
            printf("%s\n", NO_MORE_QUESTIONS_MSG);
            client->question_size = sizeof(NO_MORE_QUESTIONS_MSG);
            strncpy(client->question, NO_MORE_QUESTIONS_MSG, client->question_size);
            return;
        }
        client->question[(client->question_size)++] = c;
    }

    client->question[client->question_size - 1] = '\0';
}

int find_free(quiz_client_t *clients, int max_clients)
{
    for (int i = 0; i < max_clients; ++i)
    {
        if (clients[i].isFree)
            return i;
    }

    return -1;
}

void delete_client(quiz_client_t *client, fd_set *pbase_wfds, int *pclients_count)
{
    client->isFree = 1;
    FD_CLR(client->client_fd, pbase_wfds);
    (*pclients_count)--;
}

void send_ending_msg(quiz_client_t *clients, int max_clients)
{
    char buf[] = END_MSG;

    for (int i = 0; i < max_clients; ++i)
    {
        if (!clients[i].isFree)
        {
            if (bulk_write(clients[i].client_fd, buf, strlen(buf)) < 0 && errno != EPIPE && errno != ECONNRESET)
                ERR("write");

            if (TEMP_FAILURE_RETRY(close(clients[i].client_fd)) < 0)
                ERR("close");
        }
    }
}

void sighandler(int sig)
{
    last_signal = sig;
    if (sig == SIGUSR1)
        no_more_clients = 1;
}