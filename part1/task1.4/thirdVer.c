#define _GNU_SOURCE

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include "thread.h"

void cleanup(void* arg) {
    free(arg);
    printf("Memory freed\n");
}

void* endless(void* arg) {
    char* string = (char*)malloc(13 * sizeof(char));

    if (string == NULL) {
        fprintf(stderr, "Error allocating memory\n");
    }

    strcpy(string, "Hello, World");

    pthread_cleanup_push(cleanup, string);

    while(ENDLESS) {
        printf("%s\n", string);
    }

    pthread_cleanup_pop(CLEAN);

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