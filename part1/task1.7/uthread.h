#pragma once

#define _GNU_SOURCE

#include <sched.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <ucontext.h>
#include <sys/types.h>
#include <sys/syscall.h> 
#include <linux/futex.h>
#include <sys/resource.h>

#define PAGE_SIZE 2048
#define STACK_SIZE (PAGE_SIZE * 4)

#define TRUE 1
#define FALSE 0

#define EMPTY 0
#define ENDLESS 1
#define MAX_THREADS 8

#define SHEDULE_ERROR 1
#define MAP_ERROR 2
#define NO_VALID_ERROR 3
#define NULL_ERROR 4

enum status {
    READY,
    RUNNING,
    FINISHED,
    JOINED
};

typedef void *(*start_routine_t)(void*);

typedef struct uthread {
    int tid;
    void *arg;
    void *retval;
    ucontext_t ucntx;
    int thread_status;
    start_routine_t start_routine;
} uthread_t; 

typedef struct pair {
    char* key;
    int value;
} pair_t;

int uthread_shedule(void);

int uthread_create(uthread_t **uthread, void *(*start_routine)(void *), void *arg);
int uthread_join(uthread_t *uthread, void **retval);
