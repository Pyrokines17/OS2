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
  
    printf("Hello from tid: %d (%ld)\n", gettid(), (unsigned long)tid);
    return NULL;
}

int main(void) {
    int err;
    
    pthread_t thread;
    pthread_attr_t attr;

    err = pthread_attr_init(&attr);

    if (err != EXIT_SUCCESS) {
        fprintf(stderr, "Error initializing thread attributes: %s\n", strerror(err));
        return ATTR_INIT_ERROR;
    }

    err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    if (err != EXIT_SUCCESS) {
        fprintf(stderr, "Error setting thread attributes: %s\n", strerror(err));
        return ATTR_SET_ERROR;
    }

    while (ENDLESS) {
        err = pthread_create(&thread, &attr, printTid, NULL);

        if (err != EXIT_SUCCESS) {
            fprintf(stderr, "Error creating thread: %s\n", strerror(err));
            return CREATE_ERROR;
        }
    }

    err = pthread_attr_destroy(&attr);

    if (err != EXIT_SUCCESS) {
        fprintf(stderr, "Error destroying thread attributes: %s\n", strerror(err));
        return ATTR_DESTROY_ERROR;
    }

    return EXIT_SUCCESS;
}