/*
 * =====================================================================================
 *
 *       Filename:  main.c
 *
 *    Description:  Simple Example for Linux Timer and Epoll
 *                  The idea is to create a random number of messages via a Linux timer.
 *                  Then this should be printed with epoll.
 *
 *        Version:  1.0
 *        Created:  29.12.2020 16:15:46
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Patrick Erdmann
 *   Organization:
 *
 * =====================================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>

static int i = 0;

void _timer_signal(union sigval val) {
    printf("T\n");
}

static void _init_timer() {
    timer_t _timer;

    pthread_attr_t attr;
    pthread_attr_init( &attr );

    struct sched_param parm;
    parm.sched_priority = 255;
    pthread_attr_setschedparam(&attr, &parm);

    struct sigevent sig;
    sig.sigev_notify = SIGEV_THREAD;
    sig.sigev_notify_function = _timer_signal;
    sig.sigev_value.sival_int = 20;
    sig.sigev_notify_attributes = &attr;

    int ret = timer_create(CLOCK_REALTIME, &sig, &_timer);

    if (ret == 0) {
        struct itimerspec in, out;
        in.it_value.tv_sec = 0;
        in.it_value.tv_nsec = 1;
        in.it_interval.tv_sec = 0;
        in.it_interval.tv_nsec = 10000000; // 10 ms
        //issue the periodic timer request here.
        ret = timer_settime(_timer, 0, &in, &out);
        if(ret != 0) {
            printf("timer_settime() failed with %d\n", errno);
        }
    }
}

int main{
    _init_timer();

    sleep(1000);
}
