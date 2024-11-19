#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "thread.h"

void* printStruct(void* arg) {
    myStruct* instance = (myStruct*)arg;

    printf("First variable: %d\n", instance->firstVar);
    printf("Second variable: %s\n", instance->secondVar);

    free(instance);

    return NULL;
}

int main(void) {
    myStruct* instance = malloc(sizeof(myStruct));

    if (instance == NULL) {
        fprintf(stderr, "Error allocating memory for struct\n");
        return EXIT_FAILURE;
    }

    pthread_attr_t attr;
    pthread_t thread;
    int err;

    instance->firstVar = 1;
    instance->secondVar = "Hello, World";

    err = pthread_attr_init(&attr);

    if (err != EXIT_SUCCESS) {
        fprintf(stderr, "Error initializing thread attributes: %s\n", strerror(err));
        free(instance);
        return INIT_ATTR_ERROR;
    }

    err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    if (err != EXIT_SUCCESS) {
        fprintf(stderr, "Error setting thread attributes: %s\n", strerror(err));
        pthread_attr_destroy(&attr);
        free(instance);
        return SET_ATTR_ERROR;
    }

    err = pthread_create(&thread, &attr, printStruct, instance);

    if (err != EXIT_SUCCESS) {
        fprintf(stderr, "Error creating thread: %s\n", strerror(err));
        pthread_attr_destroy(&attr);
        free(instance);
        return CREATE_ERROR;
    }

    err = pthread_attr_destroy(&attr);

    if (err != EXIT_SUCCESS) {
        fprintf(stderr, "Error destroying thread attributes: %s\n", strerror(err));
        return DESTROY_ATTR_ERROR;
    }

    pthread_exit(NULL);

    return EXIT_SUCCESS;
}