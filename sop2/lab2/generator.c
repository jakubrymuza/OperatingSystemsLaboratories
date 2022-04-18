// Oświadczam, że niniejsza praca stanowiąca podstawę do uznania osiągnięcia efektów uczenia się z przedmiotu SOP2 została wykonana przeze mnie samodzielnie.
// Jakub Rymuza
// 305870

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <mqueue.h>
#include <pthread.h>

#define ERR(source) (fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), \
                     perror(source), kill(0, SIGKILL),               \
                     exit(EXIT_FAILURE))

#define NAME_LEN 100
#define MSG_LEN sizeof(pid_t) + 15
#define MSG_COUNT 10
#define RAND_C() rand() % ('z' - 'a') + 'a'

volatile sig_atomic_t last_signal = 0;
int sethandler(void (*f)(int), int sigNo)
{
        struct sigaction act;
        memset(&act, 0, sizeof(struct sigaction));
        act.sa_handler = f;
        if (-1 == sigaction(sigNo, &act, NULL))
                return -1;
        return 0;
}
void sig_handler(int sig)
{
        last_signal = sig;
}

void usage()
{
        fprintf(stderr, "invalid arguments\n");
        exit(EXIT_FAILURE);
}

int checkMq(char *, mqd_t *);
void initiateQ1(mqd_t, int);
void generatorWork(mqd_t, mqd_t, int, int, int);
void generateLargerMsg(char*, int, mqd_t);

int main(int argc, char **argv)
{
        if (argc != 5 && argc != 6) usage();
        int t = atoi(argv[1]);
        if (t < 1 || t > 10) usage();
        int p = atoi(argv[2]);
        if (p < 0 || p > 100) usage();

        char q1[NAME_LEN], q2[NAME_LEN];
        strncpy(q1, argv[3], NAME_LEN);
        strncpy(q2, argv[4], NAME_LEN);

        mqd_t q1_mqd, q2_mqd;
        struct mq_attr attr;
        attr.mq_maxmsg = MSG_COUNT;
        attr.mq_msgsize = MSG_LEN;
        int qExist = 1, n;

        if (argc == 6)
        {
                n = atoi(argv[5]);
                if (n > 10 || n < 1) usage();
        }
        else
        {
                // zakladam ze zawsze nalezy sprawdzic obie kolejki
                qExist &= checkMq(q1, &q1_mqd);
                qExist &= checkMq(q2, &q2_mqd);
        }

        if (sethandler(sig_handler, SIGINT))
                ERR("sethandler");
        srand(time(NULL));

        if (qExist)
        {
                if ((q1_mqd = TEMP_FAILURE_RETRY(mq_open(q1, O_RDWR | O_CREAT, 0600, &attr))) == (mqd_t)-1)
                        ERR("mq open");
                if ((q2_mqd = TEMP_FAILURE_RETRY(mq_open(q2, O_WRONLY | O_CREAT, 0600, &attr))) == (mqd_t)-1)
                        ERR("mq open");

                initiateQ1(q1_mqd, n);
                generatorWork(q1_mqd, q2_mqd, n, t, p);
        }

        mq_close(q1_mqd);
        mq_close(q2_mqd);
        return EXIT_SUCCESS;
}

int checkMq(char *q, mqd_t *mqd)
{
        if ((*mqd = mq_open(q, O_WRONLY | O_NONBLOCK)) == (mqd_t)-1)
        {
                if (errno == ENOENT)
                {
                        fprintf(stderr, "Queue %s does not exist!\n", q);
                        return 0;
                }
                else ERR("mq open");
        }

        return 1;
}

void initiateQ1(mqd_t mqd, int n)
{
        char msg[MSG_LEN];
        for (int i = 0; i < n && last_signal != SIGINT; ++i)
        {
                char a = RAND_C(), b = RAND_C(), c = RAND_C();
                if (snprintf(msg, MSG_LEN, "%d/%c%c%c", getpid(), a, b, c) < 0)
                        ERR("sprintf");
                if (mq_send(mqd, msg, MSG_LEN, 1))
                {
                        if (errno == EINTR)
                                break;
                        else
                                ERR("send");
                }
        }
}

#define CHAR_COUNT1 3
#define CHAR_COUNT2 5

void generatorWork(mqd_t q1_mqd, mqd_t q2_mqd, int n, int t, int p)
{
        char rcv[MSG_LEN];
        while (last_signal != SIGINT)
        {
                if (mq_receive(q1_mqd, rcv, MSG_LEN, NULL) < 1)
                {
                        if (errno == EINTR)
                                break;
                        else
                                ERR("receive");
                }

                sleep(t);
                if (last_signal == SIGINT)
                        break;

                generateLargerMsg(rcv, p, q2_mqd);

                if (mq_send(q1_mqd, rcv, MSG_LEN, 1))
                {
                        if (errno != EINTR)
                                ERR("send");
                }
        }
}

void generateLargerMsg(char* rcv, int p, mqd_t mqd)
{
        char msg[MSG_LEN];
        if (rand() % 100 < p)
        {
                snprintf(msg, MSG_LEN, "%d/", getpid());
                strncat(msg, strchr(rcv, '/') + 1, CHAR_COUNT1);
                int len = strlen(msg);
                msg[len] = '/';
                len++;
                for (int i = 0; i < CHAR_COUNT2 && len + i < MSG_LEN; ++i)
                        msg[len + i] = RAND_C();
                msg[len + CHAR_COUNT2] = '\0';
                if (mq_send(mqd, msg, MSG_LEN, 1))
                {
                        if (errno == EINTR)
                                ERR("send");
                }
        }
}
