#define _GNU_SOURCE

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>

#include "thread.h"

void* support(void* arg) {
    printf("Hello from support, i send: %d\n", 42);
    return (void*)42;
}

int main(void) {
    pthread_t thread;
    int status;
    int err;

    err = pthread_create(&thread, NULL, support, NULL);

    if (err != EXIT_SUCCESS) {
        fprintf(stderr, "Error creating thread: %s\n", strerror(err));
        return CREATE_ERROR;
    }

    err = pthread_join(thread, (void**)&status);

    if (err != EXIT_SUCCESS) {
        fprintf(stderr, "Error joining thread: %s\n", strerror(err));
        return JOIN_ERROR;
    }

    printf("Hello from main, i get: %d\n", status);

    return EXIT_SUCCESS;
}