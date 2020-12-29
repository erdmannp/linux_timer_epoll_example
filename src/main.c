/*
 * =====================================================================================
 *
 *       Filename:  main.c
 *
 *    Description:  Simple Example for Linux Timer and Epoll
 *                  The idea is to create a random number of messages via a Linux timer.
 *                  Then this should be printed eventbased with epoll.
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
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <mqueue.h>
#include <stddef.h>
#include <string.h>
#include <sys/epoll.h>

#define MAX_EVENTS 50
#define READ_BYTES 64

int msg_id;
int epoll_fd;
mqd_t mq;
const char* queue_name = "/ExampleMQ";

void timer_signal(union sigval val) {
	int ret;
	int random = rand() % MAX_EVENTS;

	char buf[READ_BYTES];
	char buf_tmp[READ_BYTES];
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);

	sprintf(buf, "now: %d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

	if (mq < 0) {
		perror("mq_open failed");
	}	

	for (int i = 0; i <= random; i++) {
		sprintf(buf_tmp, "NUM: %d %s", i, buf);
		ret = mq_send(mq, buf_tmp, READ_BYTES, 0);

		if (ret < 0) {
			perror("mq_send failed");
		}
	}

	if (ret < 0) {
		perror("mq_close failed");
	}
}

void _init_timer() {
	timer_t _timer;

	pthread_attr_t attr;
	pthread_attr_init( &attr );

	struct sched_param parm;
	parm.sched_priority = 255;
	pthread_attr_setschedparam(&attr, &parm);

	struct sigevent sig;
	sig.sigev_notify = SIGEV_THREAD;
	sig.sigev_notify_function = timer_signal;
	sig.sigev_value.sival_int = 0;
	sig.sigev_notify_attributes = &attr;

	int ret = timer_create(CLOCK_REALTIME, &sig, &_timer);

	if (ret == 0) {
		struct itimerspec in, out;
		in.it_value.tv_sec = 0;
		in.it_value.tv_nsec = 1;
		in.it_interval.tv_sec = 0;
		in.it_interval.tv_nsec = 10000000; // 10 ms

		ret = timer_settime(_timer, 0, &in, &out);
		if(ret < 0) {
			perror("timer_settime() failed with");
		}
	}
}

void _init_msg_queue() {
	struct mq_attr attr;
	struct sigevent sev;

	attr.mq_flags = 0;
	attr.mq_maxmsg = 1000;
	attr.mq_msgsize = READ_BYTES;
	attr.mq_curmsgs = 0;

	mq = mq_open(queue_name, O_CREAT | O_RDWR, 0644, &attr);

	if (mq < 0) {
		perror("init mq_open failed");
	}
}

void _init_epoll() {
	struct epoll_event event;

	epoll_fd = epoll_create1(0);
	event.events = EPOLLIN;

	event.data.fd = mq;
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, mq, &event);
}

int main() {
	srand(time(NULL));

	int event_count;
	struct epoll_event events[MAX_EVENTS];
	char read_buffer[READ_BYTES];
	size_t bytes_read;

	_init_msg_queue();
	_init_epoll();
	_init_timer();

	while(true) {
		event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
		printf("%d ready events\n", event_count);
		for(int event_index = 0; event_index < event_count; event_index++) {
			bytes_read = mq_receive(mq, read_buffer, READ_BYTES, 0);
			printf("%zd bytes read.\n", bytes_read);
			read_buffer[bytes_read] = '\0';
			printf("Read '%s'\n", read_buffer);
		}
	}
}
