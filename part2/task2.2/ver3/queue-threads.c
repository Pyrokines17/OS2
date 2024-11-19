#define _GNU_SOURCE

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>

#include <sched.h>
#include <pthread.h>

#include "queue.h"

#define RED "\033[41m"
#define NOCOLOR "\033[0m"

void set_cpu(int n) {
	int err;
	cpu_set_t cpuset;
	pthread_t tid = pthread_self();

	CPU_ZERO(&cpuset);
	CPU_SET(n, &cpuset);

	err = pthread_setaffinity_np(tid, sizeof(cpu_set_t), &cpuset);

	if (err != EXIT_SUCCESS) {
		fprintf(stderr, "set_cpu: pthread_setaffinity failed for cpu %d\n", n);
		return;
	}

	printf("set_cpu: set cpu %d\n", n);
}

void *reader(void *arg) {
	int expected = 0, stat;

	syncQueue_t* syncQueue = (syncQueue_t*)arg;
	printf("reader [%d %d %d]\n", getpid(), getppid(), gettid());

	set_cpu(1);

	stat = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "reader: pthread_setcancelstate() error id 1: %s\n", strerror(stat));
		return NULL;
	}

	while (ENDLESS) {
		stat = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

		if (stat != EXIT_SUCCESS) {
			fprintf(stderr, "reader: pthread_setcancelstate() error id 2: %s\n", strerror(stat));
			return NULL;
		}

		pthread_testcancel();

		stat = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

		if (stat != EXIT_SUCCESS) {
			fprintf(stderr, "reader: pthread_setcancelstate() error id 3: %s\n", strerror(stat));
			return NULL;
		}

		int val = -1;

		int ok = queue_get(syncQueue->queue, &val);

		if (ok == FALSE) {
			continue;
		}

		if (expected != val) {
			printf(RED"ERROR: get value is %d but expected - %d" NOCOLOR "\n", val, expected);
		}

		expected = val + 1;
	}

	return NULL;
}

void *writer(void *arg) {
	int i = 0, stat;
	syncQueue_t* syncQueue = (syncQueue_t*)arg;
	printf("writer [%d %d %d]\n", getpid(), getppid(), gettid());

	set_cpu(2);

	stat = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "reader: pthread_setcancelstate() error id 1: %s\n", strerror(stat));
		return NULL;
	}

	while (ENDLESS) {
		stat = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

		if (stat != EXIT_SUCCESS) {
			fprintf(stderr, "writer: pthread_setcancelstate() error id 2: %s\n", strerror(stat));
			return NULL;
		}

		pthread_testcancel();

		stat = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

		if (stat != EXIT_SUCCESS) {
			fprintf(stderr, "writer: pthread_setcancelstate() error id 3: %s\n", strerror(stat));
			return NULL;
		}

		int ok = queue_add(syncQueue->queue, i);

		if (ok == FALSE) {
			continue;
		}

		i++;
	}

	return NULL;
}

int main() {
	pthread_t tids[2];
	queue_t *q;
	int err;

	printf("main [%d %d %d]\n", getpid(), getppid(), gettid());

	q = queue_init(1000000);

	syncQueue_t syncQueue;
	syncQueue.queue = q;

	err = pthread_create(&tids[0], NULL, reader, &syncQueue);
	
	if (err != EXIT_SUCCESS) {
		fprintf(stderr, "main: pthread_create() failed: %s\n", strerror(err));
		return MAIN_ERROR;
	}

	sched_yield();

	err = pthread_create(&tids[1], NULL, writer, &syncQueue);
	
	if (err != EXIT_SUCCESS) {
		fprintf(stderr, "main: pthread_create() failed: %s\n", strerror(err));
		return MAIN_ERROR;
	}

	sleep(10);

	for (int i = 0; i < 2; ++i) {
		err = pthread_cancel(tids[i]);

		if (err != EXIT_SUCCESS) {
			fprintf(stderr, "main: pthread_cancel() failed: %s\n", strerror(err));
			return MAIN_ERROR;
		}
	}

	for (int i = 0; i < 2; ++i) {
		err = pthread_join(tids[1-i], NULL);

		if (err != EXIT_SUCCESS) {
			fprintf(stderr, "main: pthread_join() failed: %s\n", strerror(err));
			return MAIN_ERROR;
		}
	}

	err = queue_destroy(q);

	if (err != EXIT_SUCCESS) {
		fprintf(stderr, "main: queue_destroy() failed: %s\n", strerror(err));
		return MAIN_ERROR;
	}

	return EXIT_SUCCESS;
}
