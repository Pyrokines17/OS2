#define _GNU_SOURCE

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

#include "thread.h"

int loop = TRUE;

void sigint_handler(int sig) {
    if (sig == SIGINT) {
        printf("Thread INT: SIGINT received\n");
        loop = FALSE;
    } else {
        printf("Thread INT: unexpected signal received\n");
    }
}

void* threadINT(void* arg) {
    struct sigaction act;
    int res;

    act.sa_handler = sigint_handler;

    res = sigemptyset(&act.sa_mask);

    if (res != EXIT_SUCCESS) {
        fprintf(stderr, "Error thrINT emptying signal set: %s\n", strerror(res));
        return NULL;
    }

    act.sa_flags = 0;

    res = sigaction(SIGINT, &act, NULL);

    if (res != EXIT_SUCCESS) {
        fprintf(stderr, "Error thrINT setting signal action: %s\n", strerror(res));
        return NULL;
    }

    printf("Thread INT: SIGINT handler set\n");

    while (loop) {
        sleep(1);
    }

    return NULL;
}

void* threadQUIT(void* arg) {
    sigset_t set;
    int res, sig;
    
    res = sigemptyset(&set);

    if (res != EXIT_SUCCESS) {
        fprintf(stderr, "Error thrQUIT emptying signal set: %s\n", strerror(res));
        return NULL;
    }

    res = sigaddset(&set, SIGQUIT);

    if (res != EXIT_SUCCESS) {
        fprintf(stderr, "Error thrQUIT adding signal to set: %s\n", strerror(res));
        return NULL;
    }

    res = pthread_sigmask(SIG_UNBLOCK, &set, NULL);

    if (res != EXIT_SUCCESS) {
        fprintf(stderr, "Error thrQUIT unblocking signal: %s\n", strerror(res));
        return NULL;
    }

    printf("Thread QUIT: wait SIGQUIT with using sigwait\n");

    res = sigwait(&set, &sig);

    if (res != EXIT_SUCCESS) {
        fprintf(stderr, "Error thrQUIT waiting for signal: %s\n", strerror(res));
        return NULL;
    }

    if (sig == SIGQUIT) {
        printf("Thread QUIT: SIGQUIT received\n");
    } else {
        printf("Thread QUIT: unexpected signal received\n");
    }

    return NULL;
}

int main(void) {
    pthread_t tid1, tid2;
    sigset_t set;
    int res;

    res = pthread_create(&tid1, NULL, threadQUIT, NULL);

    if (res != EXIT_SUCCESS) {
        fprintf(stderr, "Error creating thread QUIT: %s\n", strerror(res));
        return CREATE_ERROR;
    }

    res = pthread_create(&tid2, NULL, threadINT, NULL);

    if (res != EXIT_SUCCESS) {
        fprintf(stderr, "Error creating thread INT: %s\n", strerror(res));
        return CREATE_ERROR;
    }
    
    res = sigfillset(&set);

    if (res != EXIT_SUCCESS) {
        fprintf(stderr, "Error thrMAIN filling signal set: %s\n", strerror(res));
        return EXIT_FAILURE;
    }

    res = pthread_sigmask(SIG_BLOCK, &set, NULL);

    if (res != EXIT_SUCCESS) {
        fprintf(stderr, "Error thrMAIN setting signal mask: %s\n", strerror(res));
        return EXIT_FAILURE;
    }

    printf("Thread MAIN: all signals are blocked\n");

    res = pthread_join(tid1, NULL);

    if (res != EXIT_SUCCESS) {
        fprintf(stderr, "Error joining thread QUIT: %s\n", strerror(res));
        return JOIN_ERROR;
    }

    res = pthread_join(tid2, NULL);

    if (res != EXIT_SUCCESS) {
        fprintf(stderr, "Error joining thread INT: %s\n", strerror(res));
        return JOIN_ERROR;
    }

    return EXIT_SUCCESS;
}