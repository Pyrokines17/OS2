#define _GNU_SOURCE

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include "thread.h"

void* endless(void* arg) {
    char* string = "Hello, World!";

    while(ENDLESS) {
        printf("%s\n", string);
    }

    return NULL;
}

int main(void) {
    pthread_t thread;
    int err;

    err = pthread_create(&thread, NULL, endless, NULL);

    if (err != EXIT_SUCCESS) {
        fprintf(stderr, "Error creating thread: %s\n", strerror(err));
        return CREATE_ERROR;
    }

    err = pthread_cancel(thread);

    if (err != EXIT_SUCCESS) {
        fprintf(stderr, "Error canceling thread: %s\n", strerror(err));
        return CANCEL_ERROR;
    }

    pthread_exit(NULL);

    return EXIT_SUCCESS;
}