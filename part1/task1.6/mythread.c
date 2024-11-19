#define _GNU_SOURCE

#include <stdio.h>
#include <sched.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "mythread.h"

static int __thr_count[THR_COUNT] = {0};
static mythread_struct** __allthreads[THR_COUNT] = {NULL};
static volatile int superlock = FALSE;
static int __ind = 0;

static inline void superlock_lock() {
    while (__sync_lock_test_and_set(&superlock, 1)); 
}

static inline void superlock_unlock() {
    __sync_lock_release(&superlock);
}

int __mythread_start(void* mythread_struct_cur) {
    ((mythread_struct*)mythread_struct_cur)->retval = ((mythread_struct*)mythread_struct_cur)->fun(((mythread_struct*)mythread_struct_cur)->args);
    
    superlock_lock();
    ((mythread_struct*)mythread_struct_cur)->state = THREAD_TERMINATED;
    superlock_unlock();

    return EXIT_SUCCESS;
}

mythread_struct* __mythread_fill(void* (*fun)(void*), void* args) {
    int cur = __ind / THR_COUNT;
    int locind = __ind % THR_COUNT;

    if (__allthreads[cur] == NULL) {
        __allthreads[cur] = (mythread_struct**)malloc(sizeof(mythread_struct*) * THR_COUNT);

        if (__allthreads[cur] == NULL) {
            fprintf(stderr, "Error allocating memory\n");
            return NULL;
        }
    }

    __allthreads[cur][locind] = (mythread_struct*)malloc(sizeof(mythread_struct));

    if (__allthreads[cur][locind] == NULL) {
        fprintf(stderr, "Error allocating memory\n");
        return NULL;
    }

    __thr_count[cur]++;

    __allthreads[cur][locind]->fun = fun;
    __allthreads[cur][locind]->args = args;
    __allthreads[cur][locind]->retval = NULL;
    __allthreads[cur][locind]->state = THREAD_NOT_STARTED;
    __allthreads[cur][locind]->stack = (char*)malloc(DEFAULT_STACK_SIZE);

    if (__allthreads[cur][locind]->stack == NULL) {
        fprintf(stderr, "Error allocating memory\n");
        return NULL;
    }

    __ind++;

    return __allthreads[cur][locind];
}

int mythread_create(mythread_t* mythread, void* (*fun)(void*), void* args) {
    mythread_struct* t;
    int status;

    superlock_lock();
    t = __mythread_fill(fun, args);
    
    *mythread = __ind;

    if (t == NULL) {
        superlock_unlock();
        return FALSE;
    }

    t->state = THREAD_RUNNING;
    status = clone(__mythread_start, (void*)(t->stack + DEFAULT_STACK_SIZE), SIGCHLD | CLONE_VM | CLONE_SIGHAND | CLONE_FS | CLONE_FILES, (void *)t);

    if (status == -1) {
        superlock_unlock();
        fprintf(stderr, "Error creating thread\n");
        return FALSE;
    } else {
        t->tid = status;
        superlock_unlock();
        return TRUE;
    }
}

void free_thread(mythread_struct* t) {
    superlock_lock();
    free(t->stack);
    free(t);
    superlock_unlock();
}

int mythread_join(mythread_t mythread, void** retval) { 
    int cur, locind, status, wstatus;

    mythread--;
    cur = mythread / THR_COUNT;
    locind = mythread % THR_COUNT;

    if (mythread < __ind) {
        superlock_lock();

        switch (__allthreads[cur][locind]->state) {
            case THREAD_RUNNING:
                __allthreads[cur][locind]->jpid = getpid();
                __allthreads[cur][locind]->state = THREAD_JOIN_CALLED;

                superlock_unlock();

                while (ENDLESS) {
                    int tmpRes = waitpid(__allthreads[cur][locind]->tid, &wstatus, 0);

                    if (tmpRes == WAITPID_ERROR) {
                        fprintf(stderr, "Error waiting for child process\n");
                        status = errno;
                        break;
                    }

                    superlock_lock();
                    __allthreads[cur][locind]->state = THREAD_COLLECTED;
                    superlock_unlock();

                    if (retval != NULL) {
                        *retval = __allthreads[cur][locind]->retval;
                    }

                    free_thread(__allthreads[cur][locind]);
                    __thr_count[cur]--;

                    if (__thr_count[cur] == 0) {
                        free(__allthreads[cur]);
                        __allthreads[cur] = NULL;
                    }

                    status = EXIT_SUCCESS;
                    break;
                }
                
                break;
            case THREAD_NOT_STARTED:
            case THREAD_JOIN_CALLED:
            case THREAD_COLLECTED:
                superlock_unlock();
                status = EINVAL;
                break;
            case THREAD_TERMINATED:
                superlock_unlock();

                if (retval != NULL) {
                    *retval = __allthreads[cur][locind]->retval;
                }

                free_thread(__allthreads[cur][locind]);
                __thr_count[cur]--;

                if (__thr_count[cur] == 0) {
                    free(__allthreads[cur]);
                    __allthreads[cur] = NULL;
                }

                status = EXIT_SUCCESS;
                break;
            default:
                superlock_unlock();
                status = EINVAL;
                break;
        }

        return status;
    } else {
        return ESRCH;
    }
}
