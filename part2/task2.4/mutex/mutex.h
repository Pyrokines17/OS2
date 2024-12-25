#define _GNU_SOURCE

#include <stdatomic.h>
#include <stdbool.h>

#define NO_TID -1
#define ENDLESS 1

#define STAT_LOCK 0
#define STAT_UNLOCK 1

typedef struct _mutex {
    _Atomic int lock;
    int tid;
} mutex_t;

void mutex_init(mutex_t* m);
int mutex_lock(mutex_t* m);
int mutex_unlock(mutex_t* m);
