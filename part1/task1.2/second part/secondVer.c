#define _GNU_SOURCE

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>

#include "thread.h"

void* printTid(void* arg) {
    pthread_t tid = pthread_self();
    int err;

    err = pthread_detach(tid);

    if (err != EXIT_SUCCESS) {
        fprintf(stderr, "Error detaching thread: %s\n", strerror(err));
        return NULL;
    }
  
    printf("Hello from tid: %d (%ld)\n", gettid(), (unsigned long)tid);
    return NULL;
}

int main(void) {
    pthread_t thread;
    int err;

    while (ENDLESS) {
        err = pthread_create(&thread, NULL, printTid, NULL);

        if (err != EXIT_SUCCESS) {
            fprintf(stderr, "Error creating thread: %s\n", strerror(err));
            return CREATE_ERROR;
        }
    }

    return EXIT_SUCCESS;
}