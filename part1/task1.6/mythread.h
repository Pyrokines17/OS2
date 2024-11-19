#pragma once

#define _GNU_SOURCE

#include <setjmp.h>
#include <bits/types.h>

#define WAITPID_ERROR -1

#define BUFFER_SIZE 1024
#define SMALL_STACK_SIZE 10240
#define DEFAULT_STACK_SIZE (1024 * 1024)

#define THREAD_RUNNING 0
#define THREAD_NOT_STARTED 1
#define THREAD_TERMINATED 2
#define THREAD_JOIN_CALLED 3
#define THREAD_COLLECTED 4

#define pid_t __pid_t

#define THR_COUNT 16
#define THR_ITER 2

#define ENDLESS 1

#define FALSE 0
#define TRUE 1

typedef unsigned long int mythread_t;

typedef struct mythread_struct {
    int tid, state;
    __pid_t jpid;
    char* stack;
    void* args;
    void* retval;
    void* (*fun)(void*);
} mythread_struct;

int __mythread_start(void* mythread_struct_cur);
mythread_struct* __mythread_fill(void* (*fun)(void*), void* args);

int mythread_create(mythread_t* mythread, void* (*fun)(void*), void* args);
int mythread_join(mythread_t mythread, void** retval);
