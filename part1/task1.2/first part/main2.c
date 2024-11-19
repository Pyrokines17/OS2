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
    char* str = "hello world";
    printf("Hello from support, i send: %s\n", str);
    return (void*)str;
}

int main(void) {
    pthread_t thread;
    char* status;
    int err;

    err = pthread_create(&thread, NULL, support, NULL);

    if (err != 0) {
        printf("Error creating thread: %s\n", strerror(err));
        return CREATE_ERROR;
    }

    err = pthread_join(thread, (void**)&status);

    if (err != 0) {
        printf("Error joining thread: %s\n", strerror(err));
        return JOIN_ERROR;
    }

    printf("Hello from main, i get: %s\n", status);

    return EXIT_SUCCESS;
}