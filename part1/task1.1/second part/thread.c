#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct {
    pthread_t tid;
    int index;
} ThreadData;

int global = 20;

void *printAndChange(void *arg) {
	int local = 5;
	ThreadData *tdata = (ThreadData *)arg;

	static int staticLocal = 10;
	const int constLocal = 15;
	
	pthread_t tid = pthread_self();
	int equal = pthread_equal(tid, tdata->tid);
	
	printf("pid-%d, ppid-%d, tid-%d, pthread_self()-%lu, equal-%d\nlocal-%d-%p, staticLocal-%p, constLocal-%p, global-%d-%p\n\n",
		getpid(), getppid(), gettid(), tid, equal, local, &local, &staticLocal, &constLocal, global, &global);

	local = 25;
	global = 30;

	printf("tid-%d, pthread_self()-%lu\nlocal-%d-%p, global-%d-%p\n\n",
		gettid(), tid, local, &local, global, &global);

	scanf("%d", &local);
	
	return NULL;
}

int fiveThreads() {
	ThreadData tdata[5];
	int i, err;

	for (i = 0; i < 5; ++i) {
		tdata[i].index = i;
		err = pthread_create(&tdata[i].tid, NULL, printAndChange, &tdata[i]);

		if (err != EXIT_SUCCESS) {
			fprintf(stderr, "pthread_create() in %d thread failed: %s\n\n", i+1, strerror(err));
			return EXIT_FAILURE;
		}

		printf("pthread_create() in %d thread return: %lu\n\n", i+1, tdata[i].tid);
	}

	return EXIT_SUCCESS;
}

int main() {
	int loc, err;

	err = fiveThreads();

	if (err != EXIT_SUCCESS) {
		fprintf(stderr, "fiveThreads() failed\n");
		return EXIT_FAILURE;
	}

	scanf("%d", &loc);
	
	pthread_exit(NULL);

	return EXIT_SUCCESS;
}

