#include "mutex.h"

#include <sys/syscall.h>
#include <linux/futex.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <sched.h>

int futex(int* uaddr, int futex_op, int val, const struct timespec* timeout, int* uaddr2, int val3) {
    return syscall(SYS_futex, uaddr, futex_op, val, timeout, uaddr2, val3);
}

int futex_wait(int* fu_addr, int val) {
    int err = futex(fu_addr, FUTEX_WAIT, val, NULL, NULL, 0);

    if (err == -1 && errno != EAGAIN) {
        fprintf(stderr, "futex_wait failed: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int futex_wake(int* fu_addr) {
    int err = futex(fu_addr, FUTEX_WAKE, 1, NULL, NULL, 0);

    if (err == -1 && errno != EAGAIN) {
        fprintf(stderr, "futex_wake failed: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void mutex_init(mutex_t* m) {
    m->lock = STAT_UNLOCK;
    m->tid = NO_TID;
}

int mutex_lock(mutex_t* m) {
    while (ENDLESS) {
        int expected = STAT_UNLOCK;
        int stat = atomic_compare_exchange_strong(&m->lock, &expected, STAT_LOCK);

        if (stat == true) {
            break;
        }

        stat = futex_wait((int*)m, STAT_LOCK);

        if (stat != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }
    }

    m->tid = gettid();

    return EXIT_SUCCESS;
}

int mutex_unlock(mutex_t* m) {
    int expected = STAT_LOCK;
    int stat;

    if (m->tid != gettid()) {
        fprintf(stderr, "mutex_unlock failed: %s\n", strerror(EPERM));
        return EPERM;
    }

    stat = atomic_compare_exchange_strong(&m->lock, &expected, STAT_UNLOCK);

    if (stat == false) {
        return EXIT_SUCCESS;
    }

    stat = futex_wake((int*)m);

    if (stat != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
