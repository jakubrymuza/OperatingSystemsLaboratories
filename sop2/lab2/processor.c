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
#define MSG_COUNT 30
#define CHAR_COUNT1 3
#define CHAR_COUNT2 5
#define TIMEOUT_S 1

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

void processorWork(mqd_t, int, int);
void resendMsg(char *, int, mqd_t);
int readMsg(char *, int, mqd_t, int);

int main(int argc, char **argv)
{
        if (argc != 4)
                usage();

        int t = atoi(argv[1]);
        if (t < 1 || t > 10)
                usage();
        int p = atoi(argv[2]);
        if (p < 0 || p > 100)
                usage();

        char q2[NAME_LEN];

        strncpy(q2, argv[3], NAME_LEN);
        if (sethandler(sig_handler, SIGINT))
                ERR("sethandler");

        mqd_t q2_mqd;

        if ((q2_mqd = TEMP_FAILURE_RETRY(mq_open(q2, O_RDWR))) == (mqd_t)-1)
                ERR("mq open");

        srand(time(NULL));
        processorWork(q2_mqd, t, p);

        mq_close(q2_mqd);
        return EXIT_SUCCESS;
}

void processorWork(mqd_t mqd, int t, int p)
{
        char rcv[MSG_LEN];
        int firstMsgRead = 0;

        while (last_signal != SIGINT)
        {
                if (!readMsg(rcv, t, mqd, firstMsgRead))
                        continue;
                firstMsgRead = 1;
                if (last_signal == SIGINT)
                        return;

                sleep(t);
                if (last_signal == SIGINT)
                        return;

                printf("%s\n", rcv);
                resendMsg(rcv, p, mqd);
        }
}

int readMsg(char *rcv, int t, mqd_t mqd, int firstMsgRead)
{
        struct timespec tm;
        clock_gettime(CLOCK_REALTIME, &tm);
        tm.tv_sec += TIMEOUT_S;

        if (mq_timedreceive(mqd, rcv, MSG_LEN, NULL, &tm) < 1)
        {
                if (errno == ETIMEDOUT)
                {
                        if (firstMsgRead)
                        {
                                printf("%s\n", rcv);
                                if (t > TIMEOUT_S)
                                        sleep(t - TIMEOUT_S);
                        }
                        return 0;
                }
                else if (errno == EINTR)
                        return 1;
                else
                        ERR("receive");
        }
        return 1;
}

void resendMsg(char *rcv, int p, mqd_t mqd)
{
        char msg[MSG_LEN];
        if (rand() % 100 < p)
        {
                snprintf(msg, MSG_LEN, "%d/000/", getpid());
                strncat(msg, strchr(strchr(rcv, '/') + 1, '/') + 1, CHAR_COUNT2);
                if (mq_send(mqd, msg, MSG_LEN, 0))
                {
                        if (errno != EINTR)
                                ERR("send");
                }
        }
}