#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "thread.h"

void* printStruct(void* arg) {
    myStruct* instance = (myStruct*)arg;

    printf("First variable: %d\n", instance->firstVar);
    printf("Second variable: %s\n", instance->secondVar);

    return NULL;
}

int main(void) {
    myStruct instance;
    pthread_t thread;
    int err;

    instance.firstVar = 1;
    instance.secondVar = "Hello, World";

    err = pthread_create(&thread, NULL, printStruct, &instance);

    if (err != EXIT_SUCCESS) {
        fprintf(stderr, "Error creating thread: %s\n", strerror(err));
        return CREATE_ERROR;
    }

    err = pthread_join(thread, NULL);

    if (err != EXIT_SUCCESS) {
        fprintf(stderr, "Error joining thread: %s\n", strerror(err));
        return JOIN_ERROR;
    }

    return EXIT_SUCCESS;
}